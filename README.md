# Proxy_server
Simple Proxy server using poll with logging

## info
Simple Proxy server and test_server and Logger

- test_server : для тестирования прокси сервера. Получает сообщение и отправляет назад, если первый символ 'Q', тогда ничего не отправляет.
- Proxy : получает от пользователя и перенаправляет серверу. Сохраняет минимум информации, если что-то получено, то состояние POLLIN | POLLOUT, если все отправлено то возвращается состояние POLLIN.

## using
- ./proxy <proxy_port> <remote_host>:<remote_port>
- ./test_server - default setting 127.0.0.1:5555
