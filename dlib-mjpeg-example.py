import cv2
import urllib 
import sys
import numpy as np
import dlib
from skimage import io


stream=urllib.urlopen('http://oviso.axiscam.net/axis-cgi/mjpg/video.cgi')
#cascPath = sys.argv[1]
#faceCascade = cv2.CascadeClassifier(cascPath)

detector = dlib.get_frontal_face_detector()
win = dlib.image_window()

bytes=''
while True:
    bytes+=stream.read(1024)
    a = bytes.find('\xff\xd8')
    b = bytes.find('\xff\xd9')
    if a!=-1 and b!=-1:
        jpg = bytes[a:b+2]
        bytes= bytes[b+2:]
        i = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8),cv2.IMREAD_COLOR)
        gray = cv2.cvtColor(i, cv2.COLOR_BGR2RGB)
        
        dets = detector(i, 1)

        print("Number of faces detected: {}".format(len(dets)))
        for i, d in enumerate(dets):
            print("Detection {}: Left: {} Top: {} Right: {} Bottom: {}".format(
                i, d.left(), d.top(), d.right(), d.bottom()))

        win.clear_overlay()
        win.set_image(gray)
        win.add_overlay(dets)
