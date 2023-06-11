# Отчет ИДЗ-3 Котков Дмитрий Павлович БПИ213

ВАРИАНТ 32 ЗАДАНИЕ:


Вторая задача об Острове Сокровищ. Шайка пиратов под предводительством Джона Сильвера высадилась на берег Острова Сокровищ. Не смотря на добытую карту старого Флинта, местоположение сокровищ попрежнему остается загадкой, поэтому искать клад приходится практически на ощупь. Так как Сильвер ходит на деревянной ноге, то самому бродить по джунглям ему не с руки. Джон Сильвер поделил остров на участки, а пиратов на небольшие группы. Каждой группе поручается искать клад на нескольких участках, а сам Сильвер ждет на берегу. Группа пиратов, обшарив один участок, переходит на следующий, еще необследованный. Закончив поиски, пираты возвращаются к Сильверу и докладывают о результатах. Если какая–то из групп находит клад на одном из своих участков, она возвращается к Сильверу, который шлет попугая, инициализирующего прекращение (прерывание) всех работ. Требуется создать приложение, моделирующее действия Сильвера и пиратов.Сервер — Сильвер, Каждая из групп пиратов — клиент.





# Реализовано:

- Приложение-сервер server.c
  Эта программа иммитирует работу Сильвера: высылает своим командам карту с описанием, где лежат сокровища.
  
- Приложение-клиент client.c
  Эта программа реализует работу команд Сильвера: обыскивают местность и сообщают о нахождении клада.
  
# Сценарий выполнения:

Сервер и клиенты: клиенты подключаются к серверу с помощью функции recvfrom(), передают информацию о номере участка и получают информацию о состоянии этого участка с помощью функции sendto(). Когда клиент находит клад, он отправляет сообщение на сервер с помощью функции sendto().
Процессы и сервер: каждый клиентский процесс взаимодействует с сервером с помощью функций sendto() и recvfrom(). Он отправляет запросы на получение информации об участках и сообщает о нахождении клада. Сервер в свою очередь отправляет ответы клиентам и рассылает сообщение о нахождении клада всем клиентам.
