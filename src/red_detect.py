"""
Red Object Detector
--------------------
Captures live video from the default webcam, detects red-coloured
objects, draws a bounding box around each, and prints/overlays their
(x, y) coordinates (center of the bounding box) in real time.

Controls:
    q  -> quit

Requirements:
    pip install opencv-python numpy
"""

import cv2
import numpy as np

MIN_CONTOUR_AREA = 500

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Could not access the webcam.")
    exit()

print("Press 'q' to quit.")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Failed to grab frame.")
        break

    frame = cv2.flip(frame, 1)  # mirror view, optional
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Red wraps around the HSV hue circle (0 and 180 are both 'red'),
    # so we need two ranges and combine them.
    lower_red_1 = np.array([0, 120, 70])
    upper_red_1 = np.array([10, 255, 255])
    lower_red_2 = np.array([170, 120, 70])
    upper_red_2 = np.array([180, 255, 255])

    mask1 = cv2.inRange(hsv, lower_red_1, upper_red_1)
    mask2 = cv2.inRange(hsv, lower_red_2, upper_red_2)
    mask = cv2.bitwise_or(mask1, mask2)

    # Clean up the mask: remove small noise, fill small holes
    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.erode(mask, kernel, iterations=1)
    mask = cv2.dilate(mask, kernel, iterations=2)

    contours, _ = cv2.findContours(
        mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area < MIN_CONTOUR_AREA:
            continue

        x, y, w, h = cv2.boundingRect(cnt)
        cx, cy = x + w // 2, y + h // 2

        # Draw bounding box and center point
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.circle(frame, (cx, cy), 4, (255, 0, 0), -1)

        # Overlay coordinates on the frame
        label = f"({cx}, {cy})"
        cv2.putText(
            frame, label, (x, y - 10),
            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2
        )

        # Print to console as well
        print(f"Red object detected at: x={cx}, y={cy}, area={int(area)}")

    cv2.imshow("Red Object Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
