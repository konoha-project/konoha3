import("Syntax.RubyStyle")

void main(){
	int n = 0
	
	if 1 == 1 then
		n = 1
	end
	assert(n == 1)

	if 100 < 1 then
		n = 1
	else
		n = 2
	end
	assert(n == 2)
	
	if 4 / 2 == 2	
		n = 1
		if 1 == 1
			n = 3
		end
	end
	assert(n == 3)

	if 1 != 1
		n = 2
	elsif 1 != 1
		n = 3
	elsif true
		n = 4
	end
	assert(n == 4)

	if 1 != 1
		n = 3
	elsif 1 != 1
		n = 4
	else
		n = 5
	end
	assert(n == 5)

	if 1 == 1
		n = 6
	elsif 1 != 1
		n = 7
	else
		n = 8
	end
	assert(n == 6)

}

main();

