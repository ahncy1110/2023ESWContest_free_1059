#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import cv2
import mediapipe as mp
import numpy as np
import time
import math
import re
import os

################### MQTT ####################
def on_connect(client, userdata, flags, rc):
    # rc: result code
    print("Connected with result code " + str(rc))
    if rc == 0:
        print("connected OK")
    else:
        print("Bad connection Returned code=", rc)

def on_disconnect(client, userdata, flags, rc=0):
    print("Disconnected with result code " + str(rc))

def on_publish(client, userdata, mid):
    print("publish topic and message, pub#: ", mid)

def on_subscribe(client, obj, mid, granted_qos):
    print("Subscribe complete: " + str(mid) + ", qos: " + str(granted_qos))

def on_message(client, userdata, msg):
    # 메시지를 받았을 때 실행할 코드 작성
    print("received topic: ", msg.topic)
    if "regist" in msg.topic:
        # regist 시작할 때, 파일 초기화
        if msg.topic == "regist/0":
            global device_num
            device_num = 0
            fileClear()
        # 아두이노와 등록 기기 숫자 맞추기
        if str(device_num) in msg.topic:
            initialize()
            client.publish("registOK", msg.topic)

    elif msg.topic == "user":
        pred_device = mapping()
        print("publish pred_device to arduino")
        client.publish("device", str(pred_device))

def start_mqtt():
    broker_nano = "192.168.0.10"
    mqttc = mqtt.Client("python client")

    # 연결 및 콜백 함수 설정
    mqttc.on_connect = on_connect
    mqttc.on_message = on_message
    mqttc.on_disconnect = on_disconnect
    mqttc.on_publish = on_publish
    mqttc.on_subscribe = on_subscribe

    # connect and subscribe
    mqttc.connect(broker_nano)
    
    for i in range(20):
        topic = "regist/" + str(i)
        mqttc.subscribe(topic)

    mqttc.subscribe("user")

    return mqttc

#################################################

############### txt file(angle map) ###############
filePath = './angle_map_0605_demo.txt'

def fileClear():
    if os.path.isfile(filePath):
        os.remove(filePath)
        print("txt file clear")

def write_angleMap(head_directions):
    with open(filePath, 'a', encoding='UTF-8') as f:
        x, y = head_directions
        print("device_num: ", device_num)
        print("save x,y: ", x, y)
        f.write(f'{device_num} : {x}, {y}\n')

def read_angleMap():
    angle_map = []
    with open(filePath, "r", ) as f:
        lines = f.readlines()
        lines = [line.strip().replace("'", "\"") for line in lines]
        for line in lines:
            (_, x, y) = list(map(float, re.split(r'[:,]', line)))
            angle_map.append((x, y))
    return angle_map

#################################################

############## device mapping ###############
def euclaideanDistance(point, point1):
    x, y = point
    x1, y1 = point1
    distance = math.sqrt((x1 - x) ** 2 + (y1 - y) ** 2)
    return distance

def initialize():
    global buffer
    global device_num
    average_head_direction = np.mean(buffer, axis=0)

    write_angleMap(average_head_direction)
    device_num += 1
    print("device initializing end")

def mapping():
    angle_map = read_angleMap()
    global p
    dis_arr = [euclaideanDistance(point, p) for point in angle_map]
    print(np.min(dis_arr))
    pred_device = np.argmin(dis_arr)
    print("pred_device: ", pred_device)

    return pred_device

################################################

######## 시작 전에 필요한거 불러오기 #######
FACE = [1, 33, 61, 199, 263, 291]

mp_face_mesh = mp.solutions.face_mesh
face_mesh = mp_face_mesh.FaceMesh(min_detection_confidence=0.5, min_tracking_confidence=0.5,
                                  refine_landmarks=True, max_num_faces=2)
mp_drawing = mp.solutions.drawing_utils
drawing_spec = mp_drawing.DrawingSpec(thickness=1, circle_radius=1)

device_num = 0
head_directions = {}
buffer = []

client = start_mqtt()

cap = cv2.VideoCapture(0)
while cap.isOpened():
    # MQTT client 계속 실행
    client.loop_start()

    success, image = cap.read()
    start = time.time()

    # Flip the image horizontally for a later selfie-view display
    image = cv2.flip(image, 1)

    # To improve performance
    image.flags.writeable = False
    # Get the result
    rgb_img = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    results = face_mesh.process(rgb_img)
    # To improve performance
    image.flags.writeable = True

    img_h, img_w, img_c = image.shape
    order = 0

    # Camera internals
    focal_length = 1 * img_w
    cam_matrix = np.array([[focal_length, 0, img_h / 2],
                           [0, focal_length, img_w / 2],
                           [0, 0, 1]], dtype="double"
                          )
    # The distortion parameters
    dist_matrix = np.zeros((4, 1), dtype=np.float64)

    if results.multi_face_landmarks:
        for single_face_landmarks in results.multi_face_landmarks:
            face_3d = []
            face_2d = []
            mp_drawing.draw_landmarks(
                image=image,
                landmark_list=single_face_landmarks,
                connections=mp_face_mesh.FACEMESH_CONTOURS,
                landmark_drawing_spec=drawing_spec,
                connection_drawing_spec=drawing_spec,
            )

            for idx, lm in enumerate(single_face_landmarks.landmark):
                if idx in FACE:
                    x, y = int(lm.x * img_w), int(lm.y * img_h)
                    face_2d.append([x, y])
                    face_3d.append([x, y, lm.z])

            face_2d = np.array(face_2d, dtype=np.float64)
            face_3d = np.array(face_3d, dtype=np.float64)

        # Solve PnP face
        (_, rot_vec, trans_vec) = cv2.solvePnP(face_3d, face_2d, cam_matrix, dist_matrix, flags=cv2.SOLVEPNP_ITERATIVE)

        # Get rotational matrix
        rmat, jac = cv2.Rodrigues(rot_vec)
        angles, mtxR, mtxQ, Qx, Qy, Qz = cv2.RQDecomp3x3(rmat)

        # Get the y rotation degree
        angles = np.array(angles) * 360
        # print("angles:\n {0}".format(angles))
        x = angles[0] * 1.5
        y = angles[1]
        z = angles[2]

        p = [x, y]
        buffer.append((x, y))

        if len(buffer) > 10:
            buffer = buffer[-10:]

        # Add the text on the image
        cv2.putText(image, "x: " + str(np.round(x, 2)), (400, 50 + 200 * order), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
                    (0, 0, 255), 2)
        cv2.putText(image, "y: " + str(np.round(y, 2)), (400, 100 + 200 * order), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
                    (0, 0, 255), 2)
        cv2.putText(image, "z: " + str(np.round(z, 2)), (400, 150 + 200 * order), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
                    (0, 0, 255), 2)

        end = time.time()
        totalTime = end - start

        fps = 1 / totalTime
        # print("FPS: ", fps)

        cv2.putText(image, f'FPS: {int(fps)}', (20, 450), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 2)

    cv2.imshow('Head Pose Estimation', image)

    if cv2.waitKey(5) & 0xFF == 27:  # ESC 누를 시 종료, == ord('문자')로 교체가능
        break

cap.release()
