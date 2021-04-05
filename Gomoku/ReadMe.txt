# Gomoku Client Project
### Dev by SONTDC Bkav Corp

* Giao tiếp với Gomoku Server thông qua giao thức bất đồng bộ *WSAAsyncSelect*.
* Giao diện: Login, Logout, Home, Giao diện bàn cờ, tính điểm phía server và gửi trả kết quả thi đấu cho client

## Chạy game:
1. Chạy server trước: Build File Exe của GomokuServer.
2. Trên các máy Client chạy file Exe build ra từ Project GomokuClient: lưu ý thông tin địa chỉ IP được fix cứng trong code, nếu muốn chạy Client trên Máy nào phải fix lại địa chỉ IP tương ứng với máy ấy.
3. Tên đăng nhập và mật khẩu được lưu trong file passwords.txt được đi cùng với Server.