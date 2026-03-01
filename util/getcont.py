import numpy as np 
import cv2 

font = cv2.FONT_HERSHEY_COMPLEX 

# img = cv2.imread('arch.png', cv2.IMREAD_GRAYSCALE) 
img = cv2.imread('kid.jpg', cv2.IMREAD_GRAYSCALE) 
img = cv2.bitwise_not(img)

_, threshold = cv2.threshold(img, 110, 255, cv2.THRESH_BINARY) 

contours, _= cv2.findContours(threshold, cv2.RETR_LIST, 
                            cv2.CHAIN_APPROX_SIMPLE) 

areas = [cv2.contourArea(c) for c in contours]
max_index = np.argmax(areas)

merged = np.vstack([c[:, 0, :] for i, c in enumerate(contours) if i != max_index])

merged_centered = merged - merged.mean(axis=0)

scale = np.linalg.norm(merged_centered, axis=1).mean()
merged_normalized = merged_centered / scale
# Compute distances between consecutive points
diffs = np.diff(merged_normalized, axis=0)
dists = np.sqrt((diffs ** 2).sum(axis=1))
arc_lengths = np.concatenate([[0], np.cumsum(dists)])
total_length = arc_lengths[-1]

# Interpolate to get evenly spaced points
num_points = 512  # power of 2 works best for FFT
new_arc = np.linspace(0, total_length, num_points)
x_interp = np.interp(new_arc, arc_lengths, merged_normalized[:, 0])
y_interp = np.interp(new_arc, arc_lengths, merged_normalized[:, 1])

uniform_points = np.stack((x_interp, y_interp), axis=1)

for i in range(len(uniform_points)):
    print(f"{uniform_points[i][0]/512:.15f} {uniform_points[i][1]/-512:.15f}")


# max_index = len(contours)
# points = np.vstack([c[:, 0, :] for i, c in enumerate(contours) if i != max_index])
# points = np.vstack([c[:, 0, :] for i, c in enumerate(contours)])
#
# # cnt = contours[9]
# # points = []
# #
# # for i in range(len(contours)):
# #     cnt = contours[i]
# #     for item in cnt[:, 0, :]:
# #         buf = item
# #         points.append(buf)
# #
# # points = merged[:, 0, :]
# # print(points)
# # areas = [cv2.contourArea(c) for c in contours]
# # max_index = np.argmax(areas)
# #
# # # merge all other contour points
# # points = []
# # for i, cnt in enumerate(contours):
# #     if i == max_index:
# #         continue
# #     for item in cnt[:, 0, :]:
# #         points.append(item.tolist())
# #
# # points = np.array(points)
# # print("Total merged points:", len(points))
#
# diffs = np.diff(points, axis=0)
# dists = np.sqrt((diffs ** 2).sum(axis=1))
# arc_lengths = np.concatenate([[0], np.cumsum(dists)])
#
# total_length = arc_lengths[-1]
# num_points = 512 
# new_arc = np.linspace(0, total_length, num_points)
#
# x_interp = np.interp(new_arc, arc_lengths, points[:, 0])
# y_interp = np.interp(new_arc, arc_lengths, points[:, 1])
#
# uniform_points = np.stack((x_interp, y_interp), axis=1)
#
# center = uniform_points.mean(axis=0)  # [cx, cy]
# centered_points = uniform_points - center
#
# for i in range(len(centered_points)):
#     print(f"{centered_points[i][0]/512:.15f} {centered_points[i][1]/-512:.15f}")
