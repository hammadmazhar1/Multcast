reset
dx=1
n=2
total_box_width_relative=0.75
gap_width_relative=0.01
d_width=(gap_width_relative+total_box_width_relative)*dx/2

file1 = "multicast-unicast-baseline-comparison.dat"
outputstring = "multicast-unicast-baseline-comparison.png"
set border
set key left
set terminal png font arial 24 size 900,600

set output outputstring
set title "Multicast Unicast Comparison"

set ylabel "Throughput (Mbps)"
set xlabel ""

set xtics("PC-1" 1.25, "PC-2" 2.25, "Total" 3.25)

set boxwidth total_box_width_relative/n

set style fill transparent solid 0.85 noborder

plot file1 u 1:2 w boxes lc rgb"#00CC00" title "Unicast",\
		file1 u ($1+d_width):3 w boxes lc rgb"#448855" title "Multicast"