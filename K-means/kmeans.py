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
import tkinter as tk
from tkinter import filedialog
from PIL import Image, ImageTk
import random
import os

original_tk = None
new_tk = None
loaded_file_path = ""

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

def k_means(image, clusters, centroids, recomputation_times):
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
    
    # If the save images button is checked, create a directory and save the image
    if save_image_var.get():
        current_directory = os.path.dirname(os.path.abspath(__file__))
        save_folder = os.path.join(current_directory, "Saved")
        os.makedirs(save_folder, exist_ok=True)
        save_filename = os.path.join(save_folder, os.path.basename(image)[:-4] + f"_{clusters}_clusters.jpg")
        im.save(save_filename)
    im.save(loaded_file_path[:-4] + str(clusters) + ".jpg")

def k_means_aux(image, clusters, recomputation_times):
    centroids = generate_centroids(image, clusters)
    k_means(image, clusters, centroids, recomputation_times)

def apply_k_means():
    if original_image:
        num_clusters = int(num_clusters_var.get())
        if num_clusters >= 2:
            # Use the loaded_file_path as the image_path for saving the original image
            image_path = loaded_file_path
            
            # Save the original image to the specified image_path
            original_image.save(image_path)
            
            # Apply the k-means algorithm to the original image
            k_means_aux(image_path, num_clusters, 3)
            
            # Open the quantized image from the saved location
            new_image = Image.open(loaded_file_path[:-4] + str(num_clusters) + ".jpg")
            new_image.thumbnail((400, 400))
            
            # Create a PhotoImage object from the quantized image and store it in new_images dictionary
            new_images[num_clusters] = ImageTk.PhotoImage(new_image)
            
            # Update the image labels in the GUI with the original and quantized images
            update_image_labels()
            
            # Remove the temporary images
            os.remove(image_path)
            os.remove(loaded_file_path[:-4] + str(num_clusters) + ".jpg")

def load_image():
    global original_image, new_image, loaded_file_path

    # Open a file dialog to select an image file
    file_path = filedialog.askopenfilename()

    # Check if a file was selected
    if file_path:
        loaded_file_path = file_path
        original_image = Image.open(file_path)
        new_image = original_image.copy()

        # Set the maximum width and height for displaying the images in the GUI
        max_w = 400
        max_h = 400

        # Resize the original image to fit in the display area
        original_image.thumbnail((max_w, max_h))

        # Resize the new image (copy) to fit in the display area
        new_image.thumbnail((max_w, max_h))

        # Update the image labels in the GUI with the loaded images
        update_image_labels()

def update_image_labels():
    global original_tk, new_tk

    # Create a new PhotoImage from the original image
    original_tk = ImageTk.PhotoImage(original_image)

    # Get the number of clusters from the entry box
    num_clusters = int(num_clusters_var.get())

    # Get the quantized image (if available) corresponding to the number of clusters
    new_tk = new_images.get(num_clusters, None)

    # Configure the image labels in the GUI to display the images
    original_image_label.config(image=original_tk)
    original_image_label.image = original_tk
    new_image_label.config(image=new_tk)
    new_image_label.image = new_tk

# Main application
app = tk.Tk()
app.title("K-Means Image Segmentation by Coby Colson")
app.geometry("800x600")

### Button frame
button_frame = tk.Frame(app)
button_frame.pack(anchor="nw", padx=10, pady=10)

# Load button
load_button = tk.Button(button_frame, text="Load Image", command=load_image)
load_button.pack(side=tk.LEFT, padx=5)

### Entry subframe
entry_frame = tk.Frame(button_frame)
entry_frame.pack(side=tk.LEFT, padx=10)

# Label for number of clusters
num_clusters_label = tk.Label(entry_frame, text="Clusters:")
num_clusters_label.pack(side=tk.TOP)

# Entry field for number of clusters
num_clusters_var = tk.StringVar(value="5")
num_clusters_entry = tk.Entry(entry_frame, textvariable=num_clusters_var, width=8)
num_clusters_entry.pack(side=tk.LEFT, padx=5)

# Apply k-means button
apply_button = tk.Button(button_frame, text="Apply K-Means", command=apply_k_means)
apply_button.pack(side=tk.LEFT, padx=5)

# Save quantized image checkbox
save_image_var = tk.IntVar()
save_image_checkbox = tk.Checkbutton(button_frame, text="Save Quantized Image", variable=save_image_var)
save_image_checkbox.pack(side=tk.LEFT, padx=5, pady=10)

### Image frame
image_frame = tk.Frame(app)
image_frame.pack(anchor="nw", padx=10, pady=10)

original_image_label = tk.Label(image_frame)
original_image_label.pack(side=tk.LEFT)

new_image_label = tk.Label(image_frame)
new_image_label.pack(side=tk.LEFT)

original_image = None
new_image = None
new_images = {}

app.mainloop()