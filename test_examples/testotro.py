import cv2
cap = cv2.VideoCapture('http://192.168.10.108/axis-cgi/mjpg/video.cgi?channel=0&subtype=0')

while True:
  ret, frame = cap.read()
  cv2.imshow('Video', frame)

  if cv2.waitKey(1) == 27:
    exit(0)