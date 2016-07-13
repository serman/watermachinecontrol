import cv2
import urllib 
import sys
import numpy as np

#stream=urllib.urlopen('http://192.168.1.108/axis-cgi/h.264/video.cgi')
stream=urllib.urlopen('http://192.168.10.108/axis-cgi/mjpg/video.cgi?channel=0&subtype=0')
cascPath = sys.argv[1]
faceCascade = cv2.CascadeClassifier(cascPath)

bytes=''
while True:
    bytes+=stream.read(1024)
    a = bytes.find('\xff\xd8')
    b = bytes.find('\xff\xd9')
    if a!=-1 and b!=-1:
        jpg = bytes[a:b+2]
        bytes= bytes[b+2:]
        i = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8),cv2.IMREAD_COLOR)
        gray = cv2.cvtColor(i, cv2.COLOR_BGR2GRAY)
        faces = faceCascade.detectMultiScale(
            gray,
            scaleFactor=1.1,
            minNeighbors=5,
            minSize=(20, 20)
        )

        # Draw a rectangle around the faces
        for (x, y, w, h) in faces:
            cv2.rectangle(i, (x, y), (x+w, y+h), (0, 255, 0), 2)

        cv2.imshow('i',i)
        if cv2.waitKey(1) ==27:
            exit(0)  