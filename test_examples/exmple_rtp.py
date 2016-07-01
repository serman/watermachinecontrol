import cv2
import sys

cascPath = sys.argv[1]
faceCascade = cv2.CascadeClassifier(cascPath)

vcap = cv2.VideoCapture("rtsp://admin:agua@192.168.1.108:554/cam/realmonitor?channel=1&subtype=0&out.mjpg")

while True:
    ret, frame = vcap.read()
    cv2.imshow('VIDEO', frame)
    cv2.waitKey(1)

# When everything is done, release the capture
vcap.release()
cv2.destroyAllWindows()