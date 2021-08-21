import socket
import time
import pyautogui

max_pos = 120

def main():
  sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sock1.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  sock1.bind(("", 11411))
  sock1.listen(1)

  sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sock2.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  sock2.bind(("", 11412))
  sock2.listen(1)


  print("waiting for socket connection")
  (clientsocket1, address) = sock1.accept()
  (clientsocket2, address) = sock2.accept()
  print("Established a socket connection from %s on port %s" % (address))
  s1 = clientsocket1
  s2 = clientsocket2
  s1.settimeout(0.5)
  s2.settimeout(0.5)

  mxi, myi = pyautogui.position()
  dmx=0

  while True:
    
    mx, my = pyautogui.position()
    dmx = (mx - mxi)/10 + max_pos/2
    #dmx+=1
    if dmx > max_pos:
        dmx = max_pos 
    #print(str(dmx))
    dmy = myi - my
    dmb1 = (str(dmx)+"\n").encode('utf-8')
    dmb2 = (str(dmy)+"\n").encode('utf-8')
    print("dmx=%s,dmy=%s",str(dmx),str(dmy))
    try:
      s1.send(dmb1)
      time.sleep(0.1)
      s2.send(dmb2)
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
      s1.close()
      s2.close()



if __name__ == '__main__':
  main()
