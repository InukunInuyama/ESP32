import socket
import time
import pyautogui

def main():
  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  sock.bind(("", 11411))
  sock.listen(1)

  print("waiting for socket connection")
  (clientsocket, address) = sock.accept()
  print("Established a socket connection from %s on port %s" % (address))
  s = clientsocket
  s.settimeout(0.5)

  mxi, myi = pyautogui.position()
  dmx=0

  while True:
    
    mx, my = pyautogui.position()
    dmx = (mx - mxi)/30
    #dmx+=1
    print("dmx=%s",str(dmx))
    if dmx > 30:
        dmx=30
    #print(str(dmx))
    dmy = my - myi
    dmxb = (str(dmx)+"\n").encode('utf-8')
    try:
      s.send(dmxb)
      #print("pass1")
      #try:
        #print("pass2")
        #rcv_data = s.recv(1, 0)
        #rcv_data = s.recv(1, socket.MSG_DONTWAIT)
        #print(rcv_data)
      #except socket.timeout as e:
        #pass
        #print("pass3")

      time.sleep(0.1)
    except:
      print("error")
      s.close()



if __name__ == '__main__':
  main()
