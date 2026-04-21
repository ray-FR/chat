import socket
import sys
from tkinter import ttk
from tkinter import *
from tkinter.ttk import *
from time import *

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
GLOBAL_ERR_IND = 0

if len(sys.argv) != 3:
    print(f"Error on arg length, expected 3 args, only have {len(sys.argv)}")
    sys.exit(1)

sock = socket.socket()
sock.connect((socket.gethostbyname(str(sys.argv[1])), int(sys.argv[2])))
sock.setblocking(False)
print("Connected\n")

root = Tk()
root.geometry('1060x480')
root.title("Chat R.I")

style = ttk.Style()
style.configure("Diss.Treeview", font=("Arial", 11))

str_interaction_entry = StringVar()
str_name_entry = StringVar()

def on_action(e, type):
    global GLOBAL_ERR_IND

    if type == 1:
        sock.send((f"NAME {str_name_entry.get()}\n").encode())
        sleep(0.5)
        response()
        
        if GLOBAL_ERR_IND == 0:
            name_toplevel.destroy()
            name.config(text=str_name_entry.get())
        str_name_entry.set("")
        GLOBAL_ERR_IND = 0

    if type == 2:
        sock.send(("LIST\n").encode())

    if type == 3:
        selected_channel = channels.focus()
        sock.send((f"JOIN {channels.item(selected_channel)["text"]}\n").encode())

    if type == 4:
        selected_channel = current_channellist.focus()
        if current_channellist.item(selected_channel)["text"] in messages_history.keys():
            if messages_history[current_channellist.item(selected_channel)["text"]][1] == 1:
                sock.send((f"TALK {current_channellist.item(selected_channel)["text"]} {str_interaction_entry.get()}\n").encode())
            else:
                sock.send((f"PRIV {current_channellist.item(selected_channel)["text"]} {str_interaction_entry.get()}\n").encode())
                messages_history[(current_channellist.item(selected_channel)["text"])][0].append([f"{name['text']}: {str_interaction_entry.get()}"])
                fill_discussion(None)

            str_interaction_entry.set("")

    if type == 5:
        sock.send(("PING\n").encode())
        sleep(0.5)
        response()

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

def popup_error(err):
    print(err)
    error_toplevel = Toplevel(root)
    error_toplevel.title("ERREUR!")
    error_toplevel.grab_set()
    error_toplevel.geometry('350x140')
    error_toplevel_label = ttk.Label(error_toplevel, text=err)
    error_toplevel_button = ttk.Button(error_toplevel, text="OK", command=error_toplevel.destroy)
    error_toplevel.bind('<Return>', lambda event: error_toplevel.destroy())
    error_toplevel_label.pack(pady=(15, 5))
    error_toplevel_button.pack()
    root.wait_window(error_toplevel)

messages_history = {"Bienvenue--------": [[[f"Bienvenue sur le serveur hébergé sur {sys.argv[1]}!"],["Pour commencer à parler, sélectionnez tout simplement un canau à dans la liste des canaux à droite!"]], 0]}
participants_history = []

def response():
    global GLOBAL_ERR_IND
    global time_to_compare

    serv_response = bytes()
    
    try:
        serv_response = sock.recv(1024)
        if not serv_response:
            print("Cut by the host")
            sys.exit(1)
        
    except BlockingIOError:
        if int(time() - time_to_compare) == 15:
            print("PING")
            time_to_compare = time()
            on_action(None, 5)
            
        
        root.after(500, response)
        return
        
    decoded_serv_response = serv_response.decode()

    if "LIST" in decoded_serv_response[0:5]:
        channels.delete(*channels.get_children())
        channels_args = decoded_serv_response.split("\n")
        for i in range(1, len(channels_args) - 1):
            channels.insert('', 'end', text=channels_args[i])

    if "TALK" in decoded_serv_response[0:5]:
        split_message = decoded_serv_response.split(" ")
        messages_history[split_message[1]][0].append([f"{split_message[2]}: {' '.join(split_message[3:])}"])
        selected_channel = current_channellist.focus()
        if current_channellist.item(selected_channel)["text"] == split_message[1]:
            fill_discussion(None)

    if "PRIV" in decoded_serv_response[0:5]:
        split_message = decoded_serv_response.split(" ")
        messages_history[split_message[1]][0].append([f"{split_message[1]}: {' '.join(split_message[2:])}"])
        selected_channel = current_channellist.focus()
        if current_channellist.item(selected_channel)["text"] == split_message[1]:
            fill_discussion(None)


    if "MEMB" in decoded_serv_response[0:5]:
        members_args = decoded_serv_response.split("\n")
        
        for i in range(1, len(members_args) - 1):
            if members_args[i] in participants_history or name['text'] == members_args[i]:
                continue
            participants_history.append(members_args[i])
            participants.insert('', 'end', text=members_args[i])




    if ((decoded_serv_response)) in err_dict.keys():
        GLOBAL_ERR_IND = 1
        popup_error(f"{err_dict[decoded_serv_response]}, code d'erreur: {decoded_serv_response}")
    else:
        print(decoded_serv_response)

    time_to_compare = time()  
    root.after(500, response)

def select_channel(event):
    selected_channel = channels.focus()
    for channel in current_channellist.get_children():
        if current_channellist.item(channel)["text"] == channels.item(selected_channel)["text"]:
            return
    current_channellist.insert('', 'end', text=channels.item(selected_channel)["text"])
    messages_history[channels.item(selected_channel)["text"]] = [[], 1]
    current_channellist.selection_set(current_channellist.get_children()[-1])
    current_channellist.focus(current_channellist.get_children()[-1])
    on_action(None, 3)

def select_member(event):
    selected_member = participants.focus()
    for participant in current_channellist.get_children():
        if current_channellist.item(participant)["text"] == participants.item(selected_member)["text"]:
            return
    current_channellist.insert('', 'end', text=participants.item(selected_member)["text"])
    messages_history[participants.item(selected_member)["text"]] = [[], 2]
    current_channellist.selection_set(current_channellist.get_children()[-1])
    current_channellist.focus(current_channellist.get_children()[-1])


def fill_discussion(event):
    discussion.delete(*discussion.get_children())
    selected_channel = current_channellist.focus()
    for msg in messages_history[current_channellist.item(selected_channel)["text"]][0]:
        discussion.insert('', 'end', text=msg[0])

time_to_compare = time()

interaction_frame = ttk.Frame(root)
name = ttk.Button(root, text="unnamed", command=popup_name)
interaction_entry = ttk.Entry(root, textvariable=str_interaction_entry, width=(root.winfo_width() - 150))
interaction_entry.bind('<Return>', lambda event: on_action(event, 4))

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


current_channellist_scrollbar = ttk.Scrollbar(root)
current_channellist = ttk.Treeview(root, show="tree", yscrollcommand=current_channellist_scrollbar.set)
current_channellist.insert('', 'end', text="Bienvenue--------")
current_channellist.bind("<Double-Button-1>", fill_discussion)
current_channellist_scrollbar.config(command=current_channellist.yview)


discussion_scrollbar = ttk.Scrollbar(root)
discussion = ttk.Treeview(root, show="tree", yscrollcommand=discussion_scrollbar.set, selectmode='none', style="Diss.Treeview")
discussion_scrollbar.config(command=discussion.yview)


channels_frame_top = ttk.Frame(root)
channels_label = ttk.Label(root, text="Canaux")
channels_button = ttk.Button(root, text="⇅", width=0, command=(lambda: on_action(None, 2)))

channels_frame_main = ttk.Frame(root)
channels_scrollbar = ttk.Scrollbar(root)
channels = ttk.Treeview(root, show="tree", yscrollcommand=channels_scrollbar.set)
channels.bind("<Double-Button-1>", select_channel)
channels_scrollbar.config(command=channels.yview)


participants_label = ttk.Label(root, text="Participants")
participants_frame = ttk.Frame(root)
participants_scrollbar = ttk.Scrollbar(root)
participants = ttk.Treeview(root, show="tree", yscrollcommand=participants_scrollbar.set)
participants.bind("<Double-Button-1>", select_member)

participants_scrollbar.config(command=participants.yview)


interaction_frame.pack(side="bottom", pady=10)
name.pack(in_= interaction_frame, side="left", padx= 20)
interaction_entry.pack(in_= interaction_frame ,side="left")


current_channellist.pack(side="left",fill="y", padx=0)
current_channellist_scrollbar.pack(side="left", fill="y")

discussion.pack(side="left", expand=YES, fill="both")
discussion_scrollbar.pack(side="left", fill="y")

channels_frame_top.pack(side="top", fill="x")
channels_label.pack(in_=channels_frame_top, side="left", padx=(10))
channels_button.pack(in_=channels_frame_top, side="right", padx=(0, 20))
channels_frame_main.pack(side="top", fill="both")
channels.pack(in_=channels_frame_main, side="left", fill="y", expand=YES)
channels_scrollbar.pack(in_=channels_frame_main, side="left", fill="y")

participants_label.pack(side="top", fill="both", padx=10)
participants_frame.pack(side="top", fill="both")
participants.pack(in_=participants_frame ,side="left", fill="y", expand=YES)
participants_scrollbar.pack(in_=participants_frame ,side="left", fill="y")



response()
root.mainloop()