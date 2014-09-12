OBJ=xdfm.js

xdfm.js: xdfm.coffee
	coffee -c $^

.js.coffee:
	coffee -c $^

clean:
	@rm -f xdfm.js

scp: $(OBJ)
	scp $(OBJ) a.dc.xdf.cn:~/
	ssh a.dc.xdf.cn "scp $(OBJ) 10.200.130.197:"
