import socket
import sys
from tkinter import ttk
from tkinter import *
from tkinter.ttk import *

err_dict = {
    "ERR! 00\n" : "Erreur côté serveur",
    "ERR! 01\n" : "Commande client invalide",
    "ERR! 02\n" : "Identification nécessaire",
    "ERR! 03\n" : "Le client n'est pas membre du canal",
    "ERR! 10\n" : "Nom Invalide",
    "ERR! 11\n" : "Nom inexistant",
    "ERR! 12\n" : "Nom indisponible",
    "ERR! 20\n" : "Message invalide",
    "ERR! 21\n" : "Impossible de relayer le message",  
}
type_of_command = 1

if len(sys.argv) != 3:
    print(f"Error on arg length, expected 3 args, only have {len(sys.argv)}")
    sys.exit(1)

sock = socket.socket()
sock.connect((socket.gethostbyname(str(sys.argv[1])), int(sys.argv[2])))
sock.setblocking(False)
print("Connected\n")

def on_return(event):
    global type_of_command

    if type_of_command == 1:
        sock.send((f"NAME {str_entry.get()}\n").encode())
        

    
def response():
    serv_response = bytes()
    
    try:
        serv_response = sock.recv(1024)
        if not serv_response:
            print("Cut by the host")
            sys.exit(1)
        
    except BlockingIOError:
        
        root.after(500, response)
        return
        

    
    
    decoded_serv_response = serv_response.decode()
    if ((decoded_serv_response)) in err_dict.keys():
        print(f"{err_dict[decoded_serv_response]}, Error code: {decoded_serv_response}")
    else:
        print(decoded_serv_response)
    
    root.after(500, response)



root = Tk()
root.geometry('960x480')
root.title("Chat R.I")

str_entry = StringVar()

entry_name = ttk.Entry(root, textvariable=str_entry, width=(root.winfo_width() - 150))
entry_name.bind('<Return>', on_return)


channels_name_scrollbar = ttk.Scrollbar(root)
channels_name = ttk.Treeview(root, show="tree", yscrollcommand=channels_name_scrollbar.set)
channels_name.insert('', 'end', text="---")
channels_name_scrollbar.config(command=channels_name.yview)

discussion_scrollbar = ttk.Scrollbar(root)
discussion = ttk.Treeview(root, show="tree", yscrollcommand=discussion_scrollbar.set, selectmode='none')
discussion_scrollbar.config(command=discussion.yview)


entry_name.pack(side="bottom", pady=(10, 10))
channels_name.pack(side="left",fill="y", padx=0)
channels_name_scrollbar.pack(side="left", fill="y")
discussion.pack(side="left", expand=YES, fill="both")
discussion_scrollbar.pack(side="left", fill="y")

response()
root.mainloop()