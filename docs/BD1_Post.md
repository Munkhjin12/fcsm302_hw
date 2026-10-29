Гарчиг: IV-2-8 – Scheduling Algorithms

Код: IV-2-8
Төсөл: Scheduling Algorithms (OSC 5-р бүлэг, Programming Project)

Товч үр дүн: FCFS, SJF, Priority, Round-Robin, Priority+RR алгоритмуудыг C хэл дээр номын бүтцээр (driver, CPU.c, Makefile) хэрэгжүүлж, атомар tid болон дундаж waiting/turnaround/response time-ийн нэмэлт даалгаврыг гүйцэтгэв. Gantt chart бүхий интерактив симулятор нь параметр өөрчилж алхамчилан харуулдаг бөгөөд C програмын бодит гаралтыг уншиж Gantt зурдаг. Автомат тестийн 18 шалгалтаас 18 нь давсан. Алгоритм бүрийг гараар бодож, C болон симуляторын гаралттай тулгасан.

GitHub: https://github.com/Munkhjin12/fcsm302_hw
Онлайн симулятор: https://fcsm302hw.vercel.app
Хавсралт: IV-2-8_Report.pdf, IV-2-8_Manual.pdf

Туршиж үзэх:

Вэб: https://fcsm302hw.vercel.app нээгээд алгоритмаа сонгож Ажиллуулах дарна.
C хэсэг: төслийн хавтаснаас sh demo.sh гэж ажиллуулна (алхам бүрийн дараа Enter дарна). Автомат тест: cd src/c && make test.