import tkinter as tk
import socket
from pythonosc import dispatcher, osc_server
import threading

# OSC Configuration
HOST = "0.0.0.0"  # Listen on all available network interfaces
PORT = 9999

# Initial values
left_number = 0
right_number = 0
background_color = "green"

def enc_a_handler(address, *args):
    global left_number
    left_number = args[0]
    update_display()

def enc_b_handler(address, *args):
    global right_number
    right_number = args[0]
    update_display()

def reset_handler(address, *args):
    global background_color
    background_color = "red" if background_color == "green" else "green"
    update_display()

def update_display():
    canvas.configure(bg=background_color)
    left_label.config(text=str(left_number))
    right_label.config(text=str(right_number))

def osc_server_thread():
    disp = dispatcher.Dispatcher()
    disp.map("/encA", enc_a_handler)
    disp.map("/encB", enc_b_handler)
    disp.map("/reset", reset_handler)
    
    server = osc_server.BlockingOSCUDPServer((HOST, PORT), disp)
    server.serve_forever()

# Start OSC server in a separate thread
threading.Thread(target=osc_server_thread, daemon=True).start()

# Initialize Tkinter
root = tk.Tk()
root.title("OSC Display")
root.geometry("600x400")

canvas = tk.Canvas(root, width=600, height=400)
canvas.pack(fill="both", expand=True)
canvas.configure(bg=background_color)

left_label = tk.Label(root, text=str(left_number), font=("Arial", 50), fg="white", bg=background_color)
left_label.place(relx=0.25, rely=0.5, anchor="center")

right_label = tk.Label(root, text=str(right_number), font=("Arial", 50), fg="white", bg=background_color)
right_label.place(relx=0.75, rely=0.5, anchor="center")

root.mainloop()