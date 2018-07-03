#! /usr/bin/ruby

# Make a ramdisk

if ARGV.length != 2 then
	$stderr.puts "Usage: mkramdisk.rb <bin data file> <size> <C symbol name>"
	exit 1
end

binfile, size, csymname = ARGV

print "\#include <geekos/types.h>"
print "u8_t #{csymname}[#{size}] = {\n\t"

len = 0

File.open(binfile, "rb") do |f|
	f.each_byte do |b|
		if len >= 10 then
			print "\n\t"
			len = 0
		end
		printf("0x%x, ", b)
		len += 1
	end
end

puts "\n};"
