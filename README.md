# IV-2-8 · Scheduling Algorithms

**Хичээл:** F.CSM302 Үйлдлийн систем · Бие даалт 1
**Хуваарилалтын код:** IV-2-8 · **Төсөл:** Scheduling Algorithms (Operating System Concepts, 10th ed., Ch. 5 Programming Project)
**UI дүрслэл:** Gantt chart
**Онлайн симулятор:** https://fcsm302hw.vercel.app

## Зорилго
Номын 5 алгоритм (FCFS, SJF, Priority, Round-Robin, Priority with RR) болон нэмэлт даалгавар (tid-г атомаар олгох, дундаж turnaround / waiting / response тооцох)-ыг C хэл дээр хэрэгжүүлж, ижил алгоритмыг вэб UI дээр Gantt chart-аар алхамчилж харуулсан.

## Quick Start

**C хувилбар (Linux / macOS / WSL, gcc + make):**
```sh
cd src/c
make fcfs                          # эсвэл: sjf | priority | rr | priority_rr | make (бүгд)
./fcfs ../../tests/schedule.txt    # Round-robin: ./rr ../../tests/schedule.txt 4  (quantum заавал биш, анхны утга 10)
make test                          # автомат тест (18 шалгалт)
```

**Алхам алхмаар demo:** төслийн хавтаснаас `sh demo.sh` гэж ажиллуулна (build, таван алгоритм, буруу оролт, `make test`). Алхам бүрийн дараа Enter дарж үргэлжлүүлнэ.

**Вэб UI:** https://fcsm302hw.vercel.app нээнэ (эсвэл `src/web/index.html`-ийг хөтөч дээр нээнэ) → алгоритм сонгоод **Ажиллуулах** дарна. Өгөгдлийг хүснэгтээр засах эсвэл "schedule.txt оруулах" хэсэгт буулгаж болно. Slider эсвэл "Тоглуулах"-аар алхамчилж харна. **C-ийн бодит гаралтыг** (`./rr ../../tests/schedule.txt 4 | pbcopy`) "C-ийн гаралт оруулах" хэсэгт буулгавал UI нь тэр гаралтаас Gantt зурна (UI ↔ C холбоо).

## Оролтын формат (`schedule.txt`)
Мөр бүр `name, priority, burst`. Priority 1–10 (их тоо = өндөр), burst ≥ 1 (ms). Бүх task t = 0 үед ирнэ. `#`-ээр эхэлсэн мөр болон хоосон мөрийг алгасна.
```
T1, 4, 20
T2, 2, 25
T3, 3, 25
T4, 3, 15
T5, 10, 10
```

## Бүтэц
```
src/c/     C хэрэгжилт (driver.c, CPU.c, list.c, tasks.c, fcfs.c, sjf.c, priority.c, rr.c, priority_rr.c, Makefile)
src/web/   Gantt chart бүхий интерактив UI (index.html, ганц файл)
tests/     schedule*.txt, bad_*.txt, expected.txt, run_tests.sh, reference_check.py (Python-ийн бие даасан загвартай тулгах), tid_race_test.c (атомар tid-ийн thread туршилт)
docs/      IV-2-8_BD1_Report.pdf, IV-2-8_Manual.pdf, img/ (screenshot)
vercel.json  Vercel-д src/web-ийг статик сайт болгож байршуулах тохиргоо
.gitignore хөрвүүлсэн executable-уудыг repo-д оруулдаггүй
demo.sh    алхам алхмаар demo скрипт (sh demo.sh)
```

## Тест
`tests/expected.txt` дахь дундаж утгуудыг вэб UI-ийн (JavaScript) цөмөөр бие даан тооцсон бөгөөд C хувилбар тэдгээртэй таарна. Мөн буруу priority/burst/формат, хоосон файл, буруу quantum, байхгүй файлыг ойлгомжтой алдаагаар татгалздаг эсэхийг шалгана.

Нэмэлт: `python3 tests/reference_check.py` (Python загвартай 20 хослолоор тулгана); `gcc -O2 -pthread tests/tid_race_test.c -o tid_race_test && ./tid_race_test` (атомар tid vs энгийн `x = x + 1`).

## Хэрэглэсэн бэлэн код, сан, AI
- Гадны сан ашиглаагүй (C стандарт сан, браузерийн JavaScript).
- Номын source code download-ийн файлуудыг ашиглаагүй; бүтцийг (driver / CPU.c run() / list / Makefile) номын тайлбарын дагуу өөрсдөө бичсэн.
- AI (Claude, Anthropic) кодын эхний хувилбар, тест,  заавар бичихэд ашиглагдсан. 