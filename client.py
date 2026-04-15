import socket
import sys
import select



if len(sys.argv) != 3:
    print(f"Error on arg length, expected 3 args, only have {len(sys.argv)}")
    sys.exit(1)

sock = socket.socket()
sock.connect((socket.gethostbyname(str(sys.argv[1])), int(sys.argv[2])))
print("Connected")

while True:
    
    user_response = str(input())
    try: 
        sock.send((user_response + "\n").encode())
    except socket.error as err:
        print(f"error {err}")
        sys.exit(1)

    
    serv_response = sock.recv(1024)
    
    if not serv_response:
        print("Cut by the host")
        sys.exit(1)

    print(serv_response.decode())

