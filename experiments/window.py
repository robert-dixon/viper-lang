import sys, pygame, os, json
from pygame import *

if len(sys.argv) != 7:
	exit()

# Get window config
theme = sys.argv[6]
with open(theme) as theme_data_file:    
    theme_data = json.load(theme_data_file)

print 

size = width, height = int(sys.argv[1]), int(sys.argv[2])
position = winx, winy = int(sys.argv[4]), int(sys.argv[5])
title = sys.argv[3]

if (winx == -1 or winy == -1):
	os.environ['SDL_VIDEO_CENTERED'] = "true"
else:
	os.environ['SDL_VIDEO_WINDOW_POS'] = "%d,%d" % (winx,winy)

pygame.init()


# theme information
backCol = theme_data['backCol'][0], theme_data['backCol'][1], theme_data['backCol'][2]
textCol = theme_data['textCol'][0], theme_data['textCol'][1], theme_data['textCol'][2]
fontPath = theme_data['fontPath']
fontSize = theme_data['fontSize']
fontBold = theme_data['fontBold']


screen = pygame.display.set_mode(size)
pygame.display.set_caption(title)

while 1:
	for event in pygame.event.get():
		if event.type == pygame.QUIT : sys.exit()

	screen.fill(backCol)

	font = pygame.font.Font(fontPath, fontSize)
	font.set_bold(fontBold);
	text = font.render("Hello There", 1, textCol)
	textpos = text.get_rect()
	textpos.left = 4
	textpos.top = 4
	screen.blit(text, textpos)
	pygame.transform.smoothscale(screen, [width, height], screen)
	screen.blit(screen, [0,0])
	pygame.display.flip()