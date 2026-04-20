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

if len(sys.argv) != 3:
    print(f"Error on arg length, expected 3 args, only have {len(sys.argv)}")
    sys.exit(1)

sock = socket.socket()
sock.connect((socket.gethostbyname(str(sys.argv[1])), int(sys.argv[2])))
sock.setblocking(False)
print("Connected\n")

def on_action(e, type):
    
    if type == 1:
        sock.send((f"NAME {str_entry.get()}\n").encode())

    if type == 2:
        sock.send(("LIST\n").encode())

def popup_name():        
    name_toplevel = Toplevel(root)
    name_toplevel.title("Votre nom")
    name_toplevel.grab_set()
    name_toplevel.geometry('400x160')
    name_toplevel_label = ttk.Label(name_toplevel, text="Quel est votre nom?")
    name_toplevel_entry = ttk.Entry(name_toplevel, textvariable=str_name_entry)
    name_toplevel_entry.bind('<Return>', lambda event: on_action(event, 1))
    name_toplevel_label.pack(pady=(30, 5))
    name_toplevel_entry.pack()
    root.wait_window(name_toplevel)

    
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

    if "LIST" in decoded_serv_response[0:5]:
        canaux.delete(*canaux.get_children())
        canaux_args = decoded_serv_response.split("\n")
        for i in range(1, len(canaux_args) - 1):
            canaux.insert('', 'end', text=canaux_args[i])



    if ((decoded_serv_response)) in err_dict.keys():
        print(f"{err_dict[decoded_serv_response]}, Error code: {decoded_serv_response}")
    else:
        print(decoded_serv_response)
    
    root.after(500, response)



root = Tk()
root.geometry('960x480')
root.title("Chat R.I")

str_interaction_entry = StringVar()
str_name_entry = StringVar()


interaction_frame = ttk.Frame(root)
name = ttk.Button(root, text="unnamed", command=popup_name)
interaction_entry = ttk.Entry(root, textvariable=str_interaction_entry, width=(root.winfo_width() - 150))
interaction_entry.bind('<Return>', lambda event: on_action(event, 1))

name_toplevel = Toplevel(root)
name_toplevel.title("Votre nom")
name_toplevel.grab_set()
name_toplevel.geometry('400x160')
name_toplevel_label = ttk.Label(name_toplevel, text="Quel est votre nom?")
name_toplevel_entry = ttk.Entry(name_toplevel, textvariable=str_name_entry)
name_toplevel_entry.bind('<Return>', lambda event: on_action(event, 1))
name_toplevel_label.pack(pady=(30, 5))
name_toplevel_entry.pack()
root.wait_window(name_toplevel)


channels_name_scrollbar = ttk.Scrollbar(root)
channels_name = ttk.Treeview(root, show="tree", yscrollcommand=channels_name_scrollbar.set)
channels_name.insert('', 'end', text="---")
channels_name_scrollbar.config(command=channels_name.yview)


discussion_scrollbar = ttk.Scrollbar(root)
discussion = ttk.Treeview(root, show="tree", yscrollcommand=discussion_scrollbar.set, selectmode='none')
discussion_scrollbar.config(command=discussion.yview)


canaux_frame_top = ttk.Frame(root)
canaux_label = ttk.Label(root, text="Canaux")
canaux_button = ttk.Button(root, text="⇅", width=0, command=(lambda: on_action(None, 2)))

canaux_frame_main = ttk.Frame(root)
canaux_scrollbar = ttk.Scrollbar(root)
canaux = ttk.Treeview(root, show="tree", yscrollcommand=canaux_scrollbar.set)
canaux_scrollbar.config(command=canaux.yview)


participants_label = ttk.Label(root, text="Participants")
participants_frame = ttk.Frame(root)
participants_scrollbar = ttk.Scrollbar(root)
participants = ttk.Treeview(root, show="tree", yscrollcommand=participants_scrollbar.set)
participants_scrollbar.config(command=participants.yview)


interaction_frame.pack(side="bottom", pady=10)
name.pack(in_= interaction_frame, side="left", padx= 20)
interaction_entry.pack(in_= interaction_frame ,side="left")


channels_name.pack(side="left",fill="y", padx=0)
channels_name_scrollbar.pack(side="left", fill="y")

discussion.pack(side="left", expand=YES, fill="both")
discussion_scrollbar.pack(side="left", fill="y")

canaux_frame_top.pack(side="top", fill="x")
canaux_label.pack(in_=canaux_frame_top, side="left", padx=(10))
canaux_button.pack(in_=canaux_frame_top, side="right", padx=(0, 20))
canaux_frame_main.pack(side="top", fill="both")
canaux.pack(in_=canaux_frame_main, side="left", fill="y", expand=YES)
canaux_scrollbar.pack(in_=canaux_frame_main, side="left", fill="y")

participants_label.pack(side="top", fill="both", padx=10)
participants_frame.pack(side="top", fill="both")
participants.pack(in_=participants_frame ,side="left", fill="y", expand=YES)
participants_scrollbar.pack(in_=participants_frame ,side="left", fill="y")



response()
root.mainloop()