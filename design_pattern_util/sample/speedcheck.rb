#!//usr/bin/ruby
require 'bigdecimal'
require 'bigdecimal/util'
require 'json'

oldresult=0.0
newresult=0.0

puts("|Time|Log|")
puts("|:---|:---|")
log=""
File.open("log.txt", "r") {|f|
	log=f.read()
}
log.each_line{|line|
	time_and_word=line.chomp.split(",")
	newresult = time_and_word[0].to_f
	if newresult != 0 then
		diff = newresult - oldresult
		puts("|#{diff}|#{time_and_word[1,time_and_word.length() - 1].to_s}|")
		oldresult = newresult
	else
		puts("|||")
	end
}
