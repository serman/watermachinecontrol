
from __future__ import print_function
import cv2
import sys
import time
import requests
import operator
import numpy as np
import os
from time import sleep
import urllib
import serial
#osc
import OSC
import threading

s=None   #Servidor osc
#arduinoSerial   = serial.Serial('COM3', 9600)

mresult = None
lastResult = None

facesIds = []

#video_capture = cv2.VideoCapture(0)
# Import library to display results
#import matplotlib.pyplot as plt
#%matplotlib inline
_url = 'https://api.projectoxford.ai/face/v1.0/detect'
#_key = '726500e04dc94de9a75e33ddd9310cfc'
_key = 'f9cf739b3848457b8ca4dfa6912f09ee'
_maxNumRetries = 2
pause=False;

def rcvOscOF():
    '''
    inicia servidor OSC
    '''
    s = OSC.OSCServer(('127.0.0.1', 12345))
    s.addDefaultHandlers()
    s.addMsgHandler("/video",rcvMessage)

    st = threading.Thread(target=s.serve_forever)
    st.setDaemon( True)
    st.start()

def rcvMessage(addr, tags, stuff, source):
      msg_string = "%s [%s] %s" % (addr, tags, str(stuff))
      sys.stdout.write("OSCServer Got: '%s' from %s\n" % (msg_string, source))
      #comprobar que el video que llega es el suyo
      print ("received ed of video")
      print (str(stuff))
      #actualizar videos
      #self.myModel.defineNextVideo();
      #self.myModel.setNextVideo(str(stuff))
      #2 Enviar nuevos videos a Processing
      #self.sendVideos()
      #cherrypy.engine.publish('websocket-broadcast', TextMessage( self.myModel.currentContentAsJson() ) )    

def sendDataLocal(pathFile):
    pathToFileInDisk = pathFile
    with open(pathToFileInDisk, 'rb') as f:
        data = f.read()

        # Draw a rectangle around the faces
        # for (x, y, w, h) in faces:
        #    cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

        # Face detection parameters

    params = {'returnFaceAttributes': 'age,gender,smile,facialHair,glasses',
              'returnFaceLandmarks': 'false'}

    headers = dict()
    headers['Ocp-Apim-Subscription-Key'] = _key
    headers['Content-Type'] = 'application/octet-stream'

    json = None

    result = processRequest(json, data, headers, params)

    # Display the resulting frame
    # cv2.imshow('Video', frame)
    if result is not None:
        # Load the original image from disk
        data8uint = np.fromstring(data, np.uint8)  # Convert string to an unsigned int array
        # img = cv2.cvtColor( cv2.imdecode( data8uint, cv2.IMREAD_COLOR ), cv2.COLOR_BGR2RGB )

        # renderResultOnImage( result, img )

        # ig, ax = plt.subplots(figsize=(15, 20))
        # cv2.imshow('Video', img)

    return result;

def sendStringImage(data):

    params = {'returnFaceAttributes': 'age,gender,smile,facialHair,glasses',
              'returnFaceLandmarks': 'false'}

    headers = dict()
    headers['Ocp-Apim-Subscription-Key'] = _key
    headers['Content-Type'] = 'application/octet-stream'

    json = None

    result = processRequest(json, data, headers, params)

    # Display the resulting frame
    # cv2.imshow('Video', frame)
    if result is not None:
        # Load the original image from disk
        data8uint = np.fromstring(data, np.uint8)  # Convert string to an unsigned int array
        # img = cv2.cvtColor( cv2.imdecode( data8uint, cv2.IMREAD_COLOR ), cv2.COLOR_BGR2RGB )

        # renderResultOnImage( result, img )

        # ig, ax = plt.subplots(figsize=(15, 20))
        # cv2.imshow('Video', img)

    return result;

def processRequest(json, data, headers, params):
    """
    Helper function to process the request to Project Oxford
    Parameters:
    json: Used when processing images from its URL. See API Documentation
    data: Used when processing image read from disk. See API Documentation
    headers: Used to pass the key information and the data type request
    """

    retries = 0
    result = None

    while True:
        response = requests.request('post', _url, json=json, data=data, headers=headers, params=params)

        if response.status_code == 429:

            print("Message: %s" % (response.json()['error']['message']))

            if retries <= _maxNumRetries:
                time.sleep(1)
                retries += 1
                continue
            else:
                print('Error: failed after retrying!')
                break

        elif response.status_code == 200 or response.status_code == 201:

            if 'content-length' in response.headers and int(response.headers['content-length']) == 0:
                result = None
            elif 'content-type' in response.headers and isinstance(response.headers['content-type'], str):
                if 'application/json' in response.headers['content-type'].lower():
                    result = response.json() if response.content else None
                elif 'image' in response.headers['content-type'].lower():
                    result = response.content
        else:
            print("Error code: %d" % (response.status_code))
            print("Message: %s" % (response.json()['error']['message']))

        break

    return result


def renderResultOnImage(result, img):
    """Display the obtained results onto the input image"""

    # mresult=result;
    print("Faces in image " + str(len(result)))

    for currFace in result:
        faceRectangle = currFace['faceRectangle']
        cv2.rectangle(img, (faceRectangle['left'], faceRectangle['top']),
                      (faceRectangle['left'] + faceRectangle['width'], faceRectangle['top'] + faceRectangle['height']),
                      color=(255, 0, 0), thickness=1)

        faceLandmarks = currFace['faceLandmarks']

        for _, currLandmark in faceLandmarks.items():
            cv2.circle(img, (int(currLandmark['x']), int(currLandmark['y'])), color=(0, 255, 0), thickness=-1, radius=1)

    for currFace in result:
        print(str(currFace['faceAttributes']))
        # print(str(currFace['faceAttributes']))
        # faceRectangle = currFace['faceRectangle']
        # faceAttributes = currFace['faceAttributes']

        # textToWrite = "%c (%d)" % ( 'M' if faceAttributes['gender']=='male' else 'F', faceAttributes['age'] )
        # cv2.putText( img, textToWrite, (faceRectangle['left'],faceRectangle['top']-15), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255), 1 )


def newest_file_in_tree(rootfolder, extension=".jpg"):
    return max(
        (os.path.join(dirname, filename)
         for dirname, dirnames, filenames in os.walk(rootfolder)
         for filename in filenames
         if filename.endswith(extension)),
        key=lambda fn: os.stat(fn).st_mtime)


cascPath = 'haarcascade_frontalface_default.xml'
faceCascade = cv2.CascadeClassifier(cascPath)



def url_to_image(url):
    # download the image, convert it to a NumPy array, and then read
    # it into OpenCV format
    resp = urllib.urlopen(url)
    imageByteArray = bytearray(resp.read())
    image = np.asarray(imageByteArray, dtype="uint8")
    image = cv2.imdecode(image, cv2.IMREAD_COLOR)

    # return the image
    return [image, str(imageByteArray)]


rcvOscOF();
while(1):
    #img = cv2.imread('http://192.168.1.108/cgi-bin/snapshot.cgi?2')
    #cv2.imshow('image',img)
    #cv2.waitKey(0)
    [image, imageString] = url_to_image('http://admin:agua@192.168.1.108/cgi-bin/snapshot.cgi?1')
#    image = cv2.imread('Lenna.png')

    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    faces = faceCascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(30, 30)
    )

    if len(faces) > 0:
        # print ("face detectd")
        mresult = sendStringImage(imageString)
#        mresult = sendDataLocal('Lenna.png')
        if mresult is not None:
            # print("mmmmmmmm not none")
            lastResult = mresult
            mresult = None

                # print(str(lastResult))
        if lastResult is not None:
            # print("lastresult not none")
            for currFace in lastResult:
                faceRectangle = currFace['faceRectangle']
                faceAttributes = currFace['faceAttributes']

                if not currFace['faceId'] in facesIds:
                    if faceAttributes['glasses'] != 'NoGlasses':
                        #arduinoSerial.write('b')
                        cv2.rectangle(image, (faceRectangle['left'], faceRectangle['top']),
                                      (faceRectangle['left'] + faceRectangle['width'],
                                       faceRectangle['top'] + faceRectangle['height']),
                                      (0, 0, 255), 2)
                    else:
                        cv2.rectangle(image, (faceRectangle['left'], faceRectangle['top']),
                                      (faceRectangle['left'] + faceRectangle['width'],
                                       faceRectangle['top'] + faceRectangle['height']),
                                      (0, 255, 0), 2)

                    facesIds[len(facesIds):] = [currFace['faceId']]
                else:
                    cv2.rectangle(image, (faceRectangle['left'], faceRectangle['top']),
                                  (faceRectangle['left'] + faceRectangle['width'],
                                   faceRectangle['top'] + faceRectangle['height']),
                                  (0, 255, 0), 2)

                textToWrite = "%c (%d)" % ('M' if faceAttributes['gender'] == 'male' else 'F', faceAttributes['age'])
                cv2.putText(image, textToWrite, (faceRectangle['left'], faceRectangle['top'] - 15),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)



    cv2.imshow("Image", image)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything is done, release the capture
cv2.destroyAllWindows()




