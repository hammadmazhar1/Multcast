reset
dx=5
n=2
total_box_width_relative=0.75
gap_width_relative=0.1
d_width=(gap_width_relative+total_box_width_relative)*dx/2

file1 = "multicast-unicast-baseline-comparison.dat"

set border

set term post font "Times-Roman,10"
set term post eps color

set output outputstring
set title "Consecutive Losses CDF"

set ylabel "Throughput (Mbps)"
set xlabel ""

set boxwidth total_box_width_relative/n

set style fill transparent solid 0.5 noborder

plot file1 u 1:2 w boxes lc rgb"blue" title "CB ON",\
		file1 u ($1+d_width):3 w boxes lc rgb"red" title "CB OFF"