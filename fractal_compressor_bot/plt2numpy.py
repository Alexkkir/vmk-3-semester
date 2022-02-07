from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
import numpy as np
import matplotlib.pyplot as plt

fig = Figure()
canvas = FigureCanvas(fig)
ax = fig.gca()

ax.text(0.0, 0.0, "Test", fontsize=45)
ax.axis('off')

canvas.draw()  # draw the canvas, cache the renderer

image = np.frombuffer(canvas.tostring_rgb(), dtype='uint8').reshape(640, 480, 3)

