

with open("rotateme.html") as f:
	with open("webpage.h", 'w') as o:
		o.write("char* mainpage = \"")
		for l in f:
			o.write("%s" % l[:-1])
		o.write("\"")