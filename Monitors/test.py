#! /usr/bin/python

f = open("log.txt","r")
list = []
n = 0
for line in f:
	if(line[0]=="B"):
		list.append(line)
	else:
		n=line
		
bufor=[]
string="B0:"

for i in range(int(n)):
	for j in range(len(list)):
		if i==int(list[j][1]):
			if(list[j][3]=="P"):
				string+=(list[j][3:8]+" ")
			else:
				string+=(list[j][3:7]+"  ")
		else:
			string+="      "
		
	bufor.append(string)
	string="B"+str(i+1)
	
for i in bufor:
	print(i)
			

