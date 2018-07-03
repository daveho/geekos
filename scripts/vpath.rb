#! /usr/bin/ruby

# Expand a makefile VPATH for given files.
# This is my first ruby script!

# $Revision: 1.1 $

if ARGV.length < 3 then
	$stderr.puts "Usage: mkdep.rb <VPATH> <files...>"
	exit 1
end

vpath, *files = ARGV

filepaths = []

files.each do |f|
	vpath.split(' ').each do |path|
		candidate = "#{path}/#{f}"
		filepaths.push(candidate) if FileTest.exists?(candidate)
	end
end

puts filepaths.join(' ')
