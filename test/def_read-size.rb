for line in ARGF
	line.chomp!
	dummy, nm, nv, nz, ne = line.split(/ +/)
	nm = Integer(nm)
	nv = Integer(nv)
	nz = Integer(nz)
	ne = Integer(ne)
	#p [nm, nv, nz, ne]
	nusd = 120
	cntla = 43 * 4
	cntlb = nm * 4 + nv * 8 + nz * 12 + ne * 6 + 4
	indx = 5 * 4 + (nm * nv * nz * ne) * 4
	#p [nusd, cntla, cntlb, indx]
	printf "%8d %3d %3d %3d %3d\n",
		nusd + cntla + cntlb + indx,
		nm, nv, nz, ne
end
