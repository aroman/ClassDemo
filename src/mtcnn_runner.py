# coding: utf-8
import time
import cv2
from pprint import pprint as pp
import mxnet as mx
from mtcnn_detector import MtcnnDetector

SHOW_VIZ = False
BENCHMARK = False

detector = MtcnnDetector(model_folder='models/mxnet-mtcnn', ctx=mx.cpu(0), num_worker=4, accurate_landmark = False)

def detect(img):

    if SHOW_VIZ:
        cv2.imshow("img", img)
        cv2.waitKey(0)

    if BENCHMARK:
        start = time.time()
        results = detector.detect_face(img)
        elapsed = time.time() - start
        print "mtcnn_runner: detect took %dms" % (int(round(elapsed * 1000)))
    else:
        results = detector.detect_face(img)


    if results is not None:
        bboxes = [list(box) for box in results[0]]
        points = [list(box) for box in results[1]]

        if SHOW_VIZ:
            draw = img.copy()
            for b in bboxes:
                cv2.rectangle(draw, (int(b[0]), int(b[1])), (int(b[2]), int(b[3])), (255, 255, 255))

            for p in points:
                for i in range(5):
                    cv2.circle(draw, (p[i], p[i + 5]), 1, (0, 0, 255), 2)
            cv2.imshow("detection result", draw)
            cv2.waitKey(0)
        else:
            return (bboxes, points)
