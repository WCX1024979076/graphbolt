import random
import os

BATCH_SIZE = 100000
SNAP_VERTEX_NUM = 875713
SNAP_EDGE_NUM = 5105039
BASE_GRAPH_RATE = 0.5
BATCH_ADD_RATE = 0.7

while 1:
  BASE_GRAPH_RATE = random.random()
  BATCH_ADD_RATE = random.random()
  SNAP_VERTEX_NUM = random.randint(1500, 100000)
  BATCH_SIZE = random.randint(1, 1000000)
  BATCH_RATE = BATCH_SIZE / (BASE_GRAPH_RATE * SNAP_EDGE_NUM)

  if BATCH_RATE > 0.05 :
    continue
  
  if int(SNAP_EDGE_NUM * BASE_GRAPH_RATE) + int(BATCH_SIZE * BATCH_ADD_RATE) > SNAP_EDGE_NUM :
    continue
  
  if int(SNAP_EDGE_NUM * BASE_GRAPH_RATE) < BATCH_SIZE - int(BATCH_SIZE * BATCH_ADD_RATE) :
    continue
  
  print("BATCH_SIZE =", BATCH_SIZE)
  print("SNAP_VERTEX_NUM =", SNAP_VERTEX_NUM)
  print("SNAP_EDGE_NUM =", SNAP_EDGE_NUM)
  print("BASE_GRAPH_RATE =", BASE_GRAPH_RATE)
  print("BATCH_ADD_RATE =", BATCH_ADD_RATE)

  if os.path.exists("/home/wangcx/tmp/notes.txt"):
    os.remove("/home/wangcx/tmp/notes.txt")
  else:
    print("The file does not exist")
  
  with open('/home/wangcx/tmp/notes.txt', 'a') as the_file:
    the_file.write("BATCH_SIZE = " + str(BATCH_SIZE) + "\n")
    the_file.write("SNAP_VERTEX_NUM = " + str(SNAP_VERTEX_NUM) + "\n")
    the_file.write("SNAP_EDGE_NUM = " + str(SNAP_EDGE_NUM) + "\n")
    the_file.write("BASE_GRAPH_RATE = " + str(BASE_GRAPH_RATE) + "\n")
    the_file.write("BATCH_ADD_RATE = " + str(BATCH_ADD_RATE) + "\n")
  
  cmd = ["make", "RunAll", "BATCH_SIZE=" + str(BATCH_SIZE), "SNAP_VERTEX_NUM=" + str(SNAP_VERTEX_NUM), "SNAP_EDGE_NUM=" + str(SNAP_EDGE_NUM), "BASE_GRAPH_RATE=" + str(BASE_GRAPH_RATE), "BATCH_ADD_RATE=" + str(BATCH_ADD_RATE)]

  os.system(" ".join(cmd))
  break