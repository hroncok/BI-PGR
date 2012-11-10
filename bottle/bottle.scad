difference() {
	union() {
		cylinder(r=100,h=350,center=true,$fn=100);
		translate([0,0,175]) cylinder(r1=100,r2=50,h=200,$fn=100);
		translate([0,0,375]) cylinder(r=50,h=100,$fn=100);
	}
	union() {
		translate([0,0,5]) cylinder(r=90,h=340,center=true,$fn=100);
		translate([0,0,175]) cylinder(r1=90,r2=40,h=200,$fn=100);
		translate([0,0,376]) cylinder(r=40,h=102,$fn=100);
	}
}
