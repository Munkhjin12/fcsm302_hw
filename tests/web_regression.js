#!/usr/bin/env node
'use strict';

const assert = require('node:assert/strict');
const { readFileSync } = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const html = readFileSync(path.join(__dirname, '..', 'src', 'web', 'index.html'), 'utf8');
const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];
const wiring = script.indexOf('/* ---------- Wiring ---------- */');
assert.ok(wiring >= 0, 'Cannot locate the web engine before UI wiring');
const { simulate, parseC, parseSchedule, validName, countSwitches, segsOf, stepAt, esc } = vm.runInNewContext(
  `${script.slice(0, wiring)}\n({ simulate, parseC, parseSchedule, validName, countSwitches, segsOf, stepAt, esc })`,
  { document: { getElementById: () => ({ tBodies: [{}] }) }, TextEncoder },
);

let passed = 0;
let failed = 0;
function test(name, check) {
  try {
    check();
    passed++;
    console.log(`PASS: ${name}`);
  } catch (error) {
    failed++;
    console.error(`FAIL: ${name}\n${error.stack}`);
  }
}

function job(name = 'A', overrides = {}) {
  return { name, arrival: 0, burst: 2, priority: 4, ...overrides };
}

function trace(name, priority, remaining, slice, from, end) {
  return `Running task = [${name}] [${priority}] [${remaining}] for ${slice} units.  (t = ${from}..${end})`;
}

const dispatches = [trace('A', 4, 2, 1, 0, 1), trace('B', 2, 1, 1, 1, 2), trace('A', 4, 1, 1, 2, 3)];
const averages = ['Average waiting time    = 1.00', 'Average turnaround time = 2.50', 'Average response time   = 0.50'];
const summary = 'Schedule summary: tasks=2 end=3';
const complete = [...dispatches, ...averages, summary].join('\n');
const taskTable = ['Task      tid  Pri  Burst   Waiting  Turnaround  Response',
  'A           1    4      2         1           3         0',
  'B           2    2      1         1           2         1'];

function accepts(text) {
  const parsed = parseC(text);
  assert.ok(parsed.sim, `Expected accepted C output, got ${JSON.stringify(parsed.errs)}`);
  assert.ok(!parsed.errs?.length, `Unexpected C parser errors: ${JSON.stringify(parsed.errs)}`);
  return parsed.sim;
}

function rejects(text) {
  const parsed = parseC(text);
  assert.ok(parsed.errs?.length, 'Expected C output to be rejected');
  assert.ok(!parsed.sim, 'Rejected C output must not return a simulation');
}

test('Sparse arrivals keep idle intervals compressed and preserve time lookup', () => {
  const result = simulate([job('A', { arrival: 1000 }), job('B', { arrival: 10000, burst: 3 })], 'fcfs', 10);
  assert.equal(result.total, 10003);
  assert.ok(result.log.length <= 5 + 2, 'Idle intervals expanded to one entry per millisecond');
  assert.equal(result.log.filter(event => event.run < 0).length, 2);
  assert.equal(stepAt(result.log, 999).run, -1);
  assert.equal(stepAt(result.log, 1000).run, 0);
  assert.equal(stepAt(result.log, result.total), null);
  const partial = segsOf(result.log, 500);
  assert.equal(partial.length, 1);
  assert.equal(partial[0].from, 0);
  assert.equal(partial[0].end, 500);
});

test('Names enforce the UTF-8 byte limit and allow literal punctuation', () => {
  for (const name of ['A'.repeat(63), 'я'.repeat(31) + 'x', 'A&B"\'<>', 'constructor', '__proto__']) {
    assert.equal(validName(name), true, name);
  }
  for (const name of ['', 'A'.repeat(64), 'я'.repeat(32), 'A,B', 'A[B]', '#comment', 'A\tB', 'A\nB', 'A\0B', 'A\x7fB']) {
    assert.equal(validName(name), false, JSON.stringify(name));
  }
  assert.equal(esc('A&B"\'<>'), 'A&amp;B&quot;&#39;&lt;&gt;');
});

test('Simulation rejects out-of-range fields, duplicate names and unknown algorithms', () => {
  for (const invalid of [{ arrival: -1 }, { arrival: 10001 }, { arrival: 0.5 }, { burst: 0 },
    { burst: 1001 }, { burst: 1.5 }, { priority: 0 }, { priority: 11 }, { name: 'bad,name' }]) {
    assert.throws(() => simulate([job('A', invalid)], 'fcfs', 10));
  }
  assert.throws(() => simulate([], 'fcfs', 10));
  assert.throws(() => simulate(Array.from({ length: 21 }, (_, i) => job(`T${i}`)), 'fcfs', 10));
  assert.throws(() => simulate([job('A'), job('A')], 'fcfs', 10));
  assert.throws(() => simulate([job()], 'missing', 10));
  for (const quantum of [0, -1, 1001, 1.5, Infinity]) assert.throws(() => simulate([job()], 'rr', quantum));
  const boundary = simulate([job('A', { burst: 1000, priority: 10, arrival: 10000 })], 'rr', 1000);
  assert.equal(boundary.rows[0].comp, 11000);
  assert.equal(simulate([job('A'), job('a')], 'fcfs', 10).rows.length, 2);
});

test('Context switches count only consecutive different active processes', () => {
  const segments = runs => runs.map(run => ({ run }));
  assert.equal(countSwitches(segments([])), 0);
  assert.equal(countSwitches(segments([0, 0, 0])), 0);
  assert.equal(countSwitches(segments([0, -1, 1])), 0);
  assert.equal(countSwitches(segments([0, 0, 1, -1, 0, 1])), 2);
  assert.equal(countSwitches(segments([-1, 0, 1, 0])), 2);
});

test('C parser accepts exact averages and an optional per-task table', () => {
  for (const input of [complete, [...dispatches, ...taskTable, ...averages, summary].join('\n')]) {
    const result = accepts(input);
    assert.equal(result.rows.length, 2);
    assert.equal(result.total, 3);
    assert.equal(result.avg.wait, 1);
    assert.equal(result.avg.tat, 2.5);
    assert.equal(result.avg.resp, 0.5);
    assert.equal(countSwitches(segsOf(result.log, result.total)), 2);
  }
});

test('C parser detects remaining-work and priority inconsistencies', () => {
  rejects(complete.replace(dispatches[2], trace('A', 4, 2, 1, 2, 3)));
  rejects(complete.replace(dispatches[2], trace('A', 5, 1, 1, 2, 3)));
  rejects(complete.replace(dispatches[0], trace('A', 11, 2, 1, 0, 1)));
});

test('C parser rejects malformed dispatch lines and timeline gaps', () => {
  rejects(complete.replace(dispatches[1], 'Running task = [B] [2] [1] for nope units.  (t = 1..2)'));
  rejects(complete.replace(dispatches[1], trace('B', 2, 1, 1, 2, 3)));
  rejects(complete.replace(dispatches[1], trace('B', 2, 1, 2, 1, 2)));
});

test('C summary detects an omitted whole task and a wrong end time', () => {
  const missingTask = [trace('A', 4, 2, 1, 0, 1), trace('A', 4, 1, 1, 1, 2), 'Schedule summary: tasks=2 end=2'].join('\n');
  rejects(missingTask);
  rejects(complete.replace(summary, 'Schedule summary: tasks=2 end=4'));
});

test('C parser rejects partial, malformed and incorrect averages', () => {
  rejects([...dispatches, averages[0], summary].join('\n'));
  rejects(complete.replace(averages[0], 'Average waiting time = 1.10'));
  rejects(complete.replace(averages[2], 'Average response time = invalid'));
});

test('C parser rejects malformed summaries', () => {
  for (const footer of ['Schedule summary: tasks=two end=3', 'Schedule summary: tasks=2',
    'Schedule summary: tasks=2 end=-3', 'Schedule summary: tasks=2 end=3 extra']) {
    rejects(complete.replace(summary, footer));
  }
});

test('C imports preserve prototype-like names and punctuation literally', () => {
  const prototypeNames = [trace('constructor', 4, 2, 2, 0, 2), trace('__proto__', 2, 1, 1, 2, 3), summary].join('\n');
  assert.equal(accepts(prototypeNames).rows.map(row => row.name).join('|'), 'constructor|__proto__');
  const literalName = 'A&B"\'<>';
  const literal = [trace(literalName, 1, 1, 1, 0, 1), 'Schedule summary: tasks=1 end=1'].join('\n');
  assert.equal(accepts(literal).rows[0].name, literalName);
});

test('C metric names may begin with output-format keywords', () => {
  const names = ['Average', 'Running task', 'Schedule summary'];
  const lines = names.map((name, i) => trace(name, 1, 1, 1, i, i + 1));
  const metrics = names.map((name, i) => `${name} ${i + 1} 1 1 ${i} ${i + 1} ${i}`);
  const input = [...lines, 'Task tid Pri Burst Waiting Turnaround Response', ...metrics,
    'Average waiting time = 1.00', 'Average turnaround time = 2.00',
    'Average response time = 1.00', 'Schedule summary: tasks=3 end=3'].join('\n');
  const result = accepts(input);
  assert.equal(result.rows.map(row => row.name).join('|'), names.join('|'));
  assert.equal(result.total, 3);
  assert.equal(result.avg.wait, 1);
  assert.equal(result.avg.tat, 2);
  assert.equal(result.avg.resp, 1);
});

test('C imports enforce process, burst and safe-time limits', () => {
  const tooMany = Array.from({ length: 21 }, (_, i) => trace(`T${i}`, 1, 1, 1, i, i + 1));
  rejects([...tooMany, 'Schedule summary: tasks=21 end=21'].join('\n'));
  rejects([trace('A', 1, 1001, 1001, 0, 1001), 'Schedule summary: tasks=1 end=1001'].join('\n'));
  rejects([trace('A', 1, 1, 1, 0, Number.MAX_SAFE_INTEGER), `Schedule summary: tasks=1 end=${Number.MAX_SAFE_INTEGER}`].join('\n'));
  const maximumBurst = [trace('A', 1, 1000, 1000, 0, 1000), 'Schedule summary: tasks=1 end=1000'].join('\n');
  assert.equal(accepts(maximumBurst).total, 1000);
});

test('Legacy complete C dispatch logs remain readable with a warning', () => {
  const result = accepts(dispatches.join('\n'));
  assert.equal(result.rows.length, 2);
  assert.equal(result.total, 3);
  assert.ok(Array.isArray(result.warnings) && result.warnings.length > 0);
});

test('Schedule import accepts comments, whitespace and case-distinct names', () => {
  const result = parseSchedule(' # comment\n\n A , 4 , 2 \n a,2,1\n');
  assert.equal(result.rows?.length, 2, JSON.stringify(result.errs));
  for (const name of ['я'.repeat(31) + 'x', 'A&B"\'<>', 'constructor', '__proto__']) {
    assert.equal(parseSchedule(`${name}, 10, 1000`).rows?.length, 1, name);
  }
});

test('Schedule import rejects bad names, fields, duplicates and excess rows', () => {
  const invalid = ['', '# only a comment', 'A,1', 'A,1,2,3', 'A,0,2', 'A,11,2', 'A,1,0', 'A,1,1001',
    'A,1,1.5', 'A,one,2', 'A,1,2\n A ,2,3', 'A[B],1,2', 'я'.repeat(32) + ',1,2', 'A\tB,1,2',
    Array.from({ length: 21 }, (_, i) => `T${i},1,1`).join('\n')];
  for (const input of invalid) {
    const result = parseSchedule(input);
    assert.ok(result.errs?.length, `Expected schedule rejection: ${JSON.stringify(input)}`);
    assert.ok(!result.rows, 'Rejected schedule must not return rows');
  }
});

console.log(`Web regression groups: ${passed} passed, ${failed} failed`);
process.exitCode = failed ? 1 : 0;
