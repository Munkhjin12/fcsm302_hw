#!/usr/bin/env node
'use strict';

const assert = require('node:assert/strict');
const { execFileSync } = require('node:child_process');
const { readFileSync, mkdtempSync, writeFileSync, rmSync } = require('node:fs');
const { tmpdir } = require('node:os');
const path = require('node:path');
const vm = require('node:vm');

const root = path.join(__dirname, '..');
const html = readFileSync(path.join(root, 'src/web/index.html'), 'utf8');
const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];
const wiring = script.indexOf('/* ---------- Wiring ---------- */');
assert.ok(wiring >= 0, 'Cannot locate the simulator engine before UI wiring');
const { simulate, parseC, segsOf, countSwitches } = vm.runInNewContext(
  `${script.slice(0, wiring)}\n({ simulate, parseC, segsOf, countSwitches })`,
  { document: { getElementById: () => ({ tBodies: [{}] }) }, TextEncoder },
);

const algorithms = { fcfs: 'fcfs', sjf: 'sjf', priority: 'prio', rr: 'rr', priority_rr: 'prrr' };
const readJobs = file => readFileSync(file, 'utf8').split(/\r?\n/)
  .map(line => line.trim()).filter(line => line && !line.startsWith('#'))
  .map(line => {
    const [name, priority, burst] = line.split(',').map(field => field.trim());
    return { name, priority: Number(priority), burst: Number(burst), arrival: 0 };
  });

function readTimeline(output) {
  const segments = [];
  for (const m of output.matchAll(/^Running task = \[([^\]]+)\].*\(t = (\d+)\.\.(\d+)\)$/gm)) {
    const segment = { name: m[1], from: Number(m[2]), end: Number(m[3]) };
    const previous = segments.at(-1);
    assert.equal(segment.from, previous?.end ?? 0, 'C emitted a discontinuous timeline');
    if (previous?.name === segment.name) previous.end = segment.end;
    else segments.push(segment);
  }
  assert.ok(segments.length, 'C emitted no segments');
  return segments;
}

function readMetrics(output) {
  const metrics = new Map();
  for (const m of output.matchAll(/^(.+?)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*$/gm)) {
    const [, name, , priority, burst, wait, turnaround, response] = m;
    const fields = [priority, burst, wait, turnaround, response].map(Number);
    const [pri, len, waiting, tat, resp] = fields;
    assert.ok(!metrics.has(name.trim()), 'Duplicate metric row');
    metrics.set(name.trim(), [pri, len, resp, tat, waiting, tat, resp]);
  }
  return metrics;
}

let cases = 0;
function verify(algorithm, schedule, quantum) {
  const label = `${algorithm} ${path.basename(schedule)} q=${quantum}`;
  const jobs = readJobs(schedule);
  const output = execFileSync(path.join(root, 'src/c', algorithm), [schedule, String(quantum)], { encoding: 'utf8' });
  const engine = simulate(jobs, algorithms[algorithm], quantum);
  const parsed = parseC(output);
  assert.ok(parsed.sim, `${label}: import rejected the complete C output: ${JSON.stringify(parsed.errs)}`);
  const timeline = readTimeline(output);
  const metrics = readMetrics(output);
  assert.equal(metrics.size, jobs.length, `${label}: C metric row count`);
  for (const result of [engine, parsed.sim]) {
    const segments = segsOf(result.log, result.total);
    const timelineNames = segments.map(s => ({ name: result.rows.find(row => row.idx === s.run)?.name, from: s.from, end: s.end }));
    assert.deepEqual(JSON.parse(JSON.stringify(timelineNames)), timeline, `${label}: merged Gantt intervals`);
    assert.equal(result.rows.length, metrics.size, `${label}: result row count`);
    for (const row of result.rows) {
      assert.deepEqual([row.priority, row.burst, row.start, row.comp, row.wait, row.tat, row.resp],
        metrics.get(row.name), `${label}: metrics for ${row.name}`);
    }
    assert.equal(countSwitches(segments), timeline.length - 1, `${label}: context switches`);
  }
  cases++;
  return { wait: engine.avg.wait, response: engine.avg.resp, switches: timeline.length - 1 };
}

const book = path.join(__dirname, 'schedule.txt');
const ties = path.join(__dirname, 'schedule_ties.txt');
for (const algorithm of Object.keys(algorithms)) {
  verify(algorithm, book, 10);
  verify(algorithm, ties, 4);
}

const quantumTable = [];
for (let q = 1; q <= 30; q++) {
  const rr = verify('rr', book, q);
  const priorityRr = verify('priority_rr', book, q);
  verify('rr', ties, q);
  verify('priority_rr', ties, q);
  quantumTable.push({ q, rr, priorityRr });
}
assert.equal(quantumTable[0].priorityRr.switches, 33, 'Priority RR q=1 regression');
assert.equal(quantumTable[9].priorityRr.switches, 7, 'Priority RR q=10 regression');

const tmp = mkdtempSync(path.join(tmpdir(), 'schedule-parity-'));
try {
  const single = path.join(tmp, 'single.txt');
  writeFileSync(single, 'One task, 1, 1000\n');
  for (const q of [1, 1000]) verify('rr', single, q);
  const punctuation = path.join(tmp, 'literal-names.txt');
  writeFileSync(punctuation, 'A & "B", 4, 7\nconstructor, 4, 11\n__proto__, 1, 5\n');
  verify('priority_rr', punctuation, 4);
} finally {
  rmSync(tmp, { recursive: true, force: true });
}

const starvationJobs = [
  { name: 'L', priority: 1, burst: 5, arrival: 0 },
  ...Array.from({ length: 6 }, (_, i) => ({ name: `H${i + 1}`, priority: 9, burst: 10, arrival: 10 * i })),
];
const waits = Object.fromEntries(['fcfs', 'prio', 'prrr'].map(algorithm => [
  algorithm, simulate(starvationJobs, algorithm, 10).rows.find(r => r.name === 'L').wait,
]));
assert.deepEqual(waits, { fcfs: 0, prio: 60, prrr: 60 });

if (process.argv.includes('--report')) {
  console.log(JSON.stringify({ parityCases: cases, quantumTable, starvation: waits }, null, 2));
} else {
  console.log(`${cases} C / simulator / C import comparisons passed (per-task metrics, Gantt, switches).`);
  console.log('Starvation example passed: FCFS 0 ms, Priority 60 ms, Priority + RR 60 ms.');
}
