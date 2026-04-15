import socket
import sys
import select

err_dict = {
    "ERR! 00\n" : "Erreur côté serveur",
    "ERR! 01\n" : "Erreur côté serveur",
    "ERR! 02\n" : "Erreur côté serveur",
    "ERR! 03\n" : "Erreur côté serveur",
    "ERR! 10\n" : "Erreur côté serveur",
    "ERR! 11\n" : "Erreur côté serveur",
    "ERR! 12\n" : "Erreur côté serveur",
    "ERR! 20\n" : "Erreur côté serveur",
    "ERR! 21\n" : "Erreur côté serveur",
     
}

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
    decoded_serv_response = serv_response.decode()
    if ((decoded_serv_response)) in err_dict:
        print(f"{err_dict[decoded_serv_response]}, Error code: {decoded_serv_response}")
    else:
        print(decoded_serv_response)

