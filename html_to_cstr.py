

with open("webpage.h", 'w') as o:
	with open("index.html") as f:
		o.write("char* index_html = ")
		for l in f:
			o.write("\"%s\\n\"\n" % l[:-1])
		o.write("\"\";")