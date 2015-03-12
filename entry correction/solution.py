#do more error checking?
#file open to write - overwrite?
#is there a header - part of error-checking?

import sys

#Variables that can be specified
inputFile = 'user.csv'
outputFile = 'correceted_user.csv' 
errorPercent = 0.1

printable = True  	#For print argument
names = {} 		#a dictionary to keep track of names
countLines = 0 

#variables used in calculating scores
scores = {}	
numFirst = 0
numLast = 0
countFirst =0
countLast =0
aveFirst = 0
aveLast = 0
sumFirst = 0
sumLast = 0
		
swapped = 0 		#a var. to count swaps

'''Process the arguments'''
args = sys.argv


if "-np" in args:
	if len(args)>2:
		print("Extra arguments passed but that's okay")
		
	printable = False
elif len(args)>1:
	print("Invalid arguments passed but that's okay")
	

'''One and only method, to update the name counts and scores after a swap has occured'''
def update(first,last):
	firstEntry = names.get(first)
	lastEntry = names.get(last)
	
	#adjusting names and scores entries for first
	fUp = firstEntry[0]-1
	lUp = firstEntry[1]+1
	names[first] = [fUp, lUp]
	
	#first [a,b] to [a-1,b+1] it was a first name so it was in scores only last names wasn't zero
	if fUp == 0:
		if lUp>1:
			del scores[first]
	else:
		scores[first]= float(fUp)/(lUp+fUp)
	
	#adjusting scores and names entries for last
	fUp = lastEntry[0]+1
	lUp = lastEntry[1]-1
	names[last] = [fUp, lUp]
	
	#want to erase it if now lUp=0 and was in the dict - iff fUp-1 nonzero
	if lUp == 0:
		if fUp>1:
			del scores[last]
	else:
		scores[last]=float(fUp)/(lUp+fUp)




'''first run through the file
making a dictionary of the format 
(name, [#appeared as first name, #appeared as last name])
'''

f = open(inputFile, 'r')
	
for line in f:
	sLine = line.split(',',2)
	first = sLine[0]
	last = sLine[1]

	if last[len(last)-1] == '\n':
		last = last[0:len(last)-1]

	if first in names :
		count = names.get(first)
		names[first]=[count[0]+1, count[1]]
	else:
		names[first]=[1,0]	
	if last in names :
		count = names.get(last)
		names[last]=[count[0], count[1]+1]
	else:
		names[last]=[0,1]	

	countLines +=1

f.close()	



'''Running through the dictionary names to calculating 'scores' for names 
Each score is #ocurrence as first name / # total ocurrence
If seen more as a last name <0.5, ow reverse '''
for name in names:
	count = names.get(name)
	if (count[0] != 0 and count[1] !=0):
		score = float(count[0])/(count[0]+count[1])
		scores[name]= score
		if score > 0.5: 
			sumFirst += score
			countFirst +=1
			numFirst += count[1]
		else:
			sumLast += score
			countLast += 1
			numLast += count[0]
		
		
'''Doing bound adjustments'''	
		
#calculating the percentage of potential changes in the document
potential = float(numFirst+numLast)/countLines


#we want to bound the potential changes to the given errorPercent
#so we are going to try adjusting the bounds
if(countFirst>0):
	aveFirst = float(sumFirst)/countFirst
if(countLast>0):
	aveLast = float(sumLast)/countLast

#firstBound: the cut-off for being considered to become a first name
#similar for lastBound
firstBound = 0.5
lastBound = 0.5

#approximately cut into half
if(potential > 2*errorPercent):
	firstBound = aveFirst
	lastBound = aveLast
#have more relaxed bounds if err<pot<2*err - adjusted linearly related to the difference
elif(potential > errorPercent):
	firstBound = aveFirst*(1-(potential-errorPercent))
	lastBound = aveLast*(1+(potential-errorPercent))

'''Running through the file again and swapping on the run'''

f = open(inputFile, 'r')
g = open(outputFile, 'w')



for line in f:
	str = ""
	swap = False
	swapScore = 0
	sLine = line.split(',',2)
	first = sLine[0]
	last = sLine[1]

	if last[len(last)-1] == '\n':
		last = last[0:len(last)-1]

	if first in scores:
		firstSc = scores.get(first)
		if last in scores:
			lastSc = scores.get(last)
			#if they are both better at being each other and obey bounds
			if (lastSc-firstSc > firstBound-lastBound):
				swap = True
				
		#check if first is more likely to be a last name		
		elif firstSc<lastBound:		
			lastCount = names.get(last)
			#check if first name is a better last name
			if (lastCount[1]-1)< float(1)/firstSc:	
				swap = True
	elif last in scores:
		lastSc = scores.get(last)
		#check if last is more likely to be a first name	
		if lastSc>firstBound: 
			firstCount = names.get(first)
			#if last name is better at being a first name than last name
			if float(firstCount[0]-1)/(firstCount[1]+firstCount[0])<lastSc:	
				lCount = names.get(last)
				swap = True

	if swap:
		update(first,last)
		str = last+","+first+"\n"
		swapped += 1
	else:
		str = first+","+last+"\n"

	g.write(str)
	
g.close()
f.close()

'''Print statements created for debugging purposes'''
if printable:
	print(len(names),"unique names/lastnames")
	print(countLines,"entries")
	print((float(len(scores))/len(names))*100,"% of names appear both as first name and last name")
	print((float(swapped)/countLines)*100,"% entries swapped")
