# Отчет ИДЗ-3 Котков Дмитрий Павлович БПИ213

ВАРИАНТ 32 ЗАДАНИЕ:


   Вторая задача об Острове Сокровищ. Шайка пиратов под предводительством Джона Сильвера высадилась на берег Острова Сокровищ. Не смотря на добытую карту старого Флинта, местоположение сокровищ попрежнему остается загадкой, поэтому искать клад приходится практически на ощупь. Так как Сильвер ходит на деревянной ноге, то самому бродить по джунглям ему не с руки. Джон Сильвер поделил остров на участки, а пиратов на небольшие группы. Каждой группе поручается искать клад на нескольких участках, а сам Сильвер ждет на берегу. Группа пиратов, обшарив один участок, переходит на следующий, еще необследованный. Закончив поиски, пираты возвращаются к Сильверу и докладывают о результатах. Если какая–то из групп находит клад на одном из своих участков, она возвращается к Сильверу, который шлет попугая, инициализирующего прекращение (прерывание) всех работ. Требуется создать приложение, моделирующее действия Сильвера и пиратов.Сервер — Сильвер, Каждая из групп пиратов — клиент.





# Реализовано:
 - В приложении-сервере (server.c) реализовано имитирование работы команды Сильвера, которая высылает своим командам карту с описанием местонахождения сокровищ. Для обработки различных сценариев и корректного завершения работы была выбрана многопроцессорная архитектура. Для каждого клиента создается отдельный процесс, который прослушивает сообщения от этого клиента. Если команда находит клад раньше остальных, сервер отправляет соответствующее сообщение всем клиентам, а остальные процессы прерываются.

 - В приложении-клиенте (client.c) реализовано поведение команд Сильвера, которые обыскивают местность и сообщают о нахождении клада. Аналогично серверу, и здесь используется многопроцессорная архитектура. Для корректной обработки сообщений от сервера и проверки области на наличие клада, клиент запускает два дочерних процесса: один для прослушивания сообщений от сервера, другой для имитации проверки области на наличие клада.

# Сценарий выполнения:

0. Клиенты устанавливают соединение с сервером, используя передачу сообщений.
1. Сервер прослушивает подключения от всех клиентов.
2. После подключения всех клиентов, сервер отправляет каждому из них задания (массивы данных) о необходимых участках местности для обыска.
3. Клиенты начинают выполнять задания и ищут клады.
4.Если одна из команд клиента находит клад раньше других, она отправляет сообщение серверу о нахождении клада.
5. Сервер получает сообщение о найденном кладе и осуществляет рассылку всем клиентам, сообщая им о том, что клад был найден.
6. Сервер отправляет сообщение с указанием прекратить работу всем клиентам.
7. Клиенты прекращают работу и завершают свои процессы.



РЕЗУЛЬТАТЫ РАБОТЫ ПРОГРАММЫ:

1. Компилируем все файлы при помощи команд:


      gcc server.c -o server
      
      
      gcc client.c -o client
      
      
2. Запускаем сервер и клиенты:


      ./server 5001 5 3
      
      
      ./client 192.168.1.20 5001 1 5
      
      
      ./client 192.168.1.20 5001 2 5
      
      
      ./client 192.168.1.20 5001 3 5
      
      
3. Итог выполнения программы:

Запуск и вывод сервера


![alt text](https://github.com/kottng/OS_IDZ_4/blob/main/server.png)


Запуск и вывод первого клиента


![alt text](https://github.com/kottng/OS_IDZ_4/blob/main/client_1.png)


Запуск и вывод второго клиента


![alt text](https://github.com/kottng/OS_IDZ_4/blob/main/client_2.png)


Запуск и вывод третьего клиента


![alt text](https://github.com/kottng/OS_IDZ_4/blob/main/client_3.png)
