# File: kmeans.py
# Author: Coby Colson
# Description:
# This script performs the K-means clustering algorithm on an input image.
# It uses the numpy and PIL (Python Imaging Library) libraries for image processing.
# The k_means function serves as the entry point to the clustering process. 
# It calls the generate_centroids function to initialize the centroids and then iterates through the desired number of clusters,
# invoking the k_means_auxiliary function for each cluster.

import numpy as np
from PIL import Image
import random

def generate_centroids(image, num_centroids):
    im = Image.open(image)
    width, height = im.size
    centroids = []
    
    # Initialize centroids using random sampling
    for i in range(0, num_centroids):
        x = random.randrange(0, width)
        y = random.randrange(0, height)
        pixel = im.getpixel((x, y))
        centroids.append(pixel)
    
    im.close()
    return centroids

def k_means_auxiliary(image, clusters, centroids, recomputation_times):
    im = Image.open(image)
    width, height = im.size
    points = centroids[:clusters]
    new_points = [(0, 0, 0)] * len(points)
    num_points_in_cluster = [0] * len(points)
    
    for i in range(0, recomputation_times + 1):
        for x in range(0, width):
            for y in range(0, height):
                nearest_point = (-1, -1, -1)
                min_dist = 3 * (255 ** 2) # Max possible distance between two pixels ((0,0,0)->(255,255,255))
                pixel = im.getpixel((x, y))
                centroid_number = 0
                current_centroid_number = 0
                
                # Find the nearest centroid for the current pixel
                for p in points:
                    distance_sq = (pixel[0] - p[0]) ** 2 + \
                                   (pixel[1] - p[1]) ** 2 + \
                                   (pixel[2] - p[2]) ** 2
                    if distance_sq < min_dist:
                        nearest_point = p
                        min_dist = distance_sq
                        centroid_number = current_centroid_number
                    current_centroid_number += 1
                
                # Update the new centroid values and count the number of points in each cluster
                new_points[centroid_number] = tuple(np.add(new_points[centroid_number], pixel))
                num_points_in_cluster[centroid_number] += 1
                
                if i == recomputation_times:
                    # Update the pixel value with the nearest centroid color for the final iteration
                    npi = tuple(int(p) for p in nearest_point)
                    im.putpixel((x, y), npi)
        
        # Compute the new centroid values by averaging the pixel values in each cluster
        for j in range(0, len(points)):
            if num_points_in_cluster[j] != 0:
                new_points[j] = tuple(np.divide(new_points[j], num_points_in_cluster[j]))
            num_points_in_cluster[j] = 0
        
        points = new_points
        new_points = [(0, 0, 0)] * len(points)
    
    im.save(image[:-4] + str(clusters)

def k_means(image, min_clusters, max_clusters, recomputation_times):
    centroids = generate_centroids(image, max_clusters)
    for i in range(min_clusters, max_clusters):
        k_means_auxiliary(image, i, centroids, recomputation_times)

k_means('YOURIMAGENAME.jpg', 3, 11, 3)