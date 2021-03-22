#!/usr/bin/env ruby -Ku

print <<EOF
\\begin{longtable}{l|rrrp{20zw}}
\\hline
要素名 & GRIB1番号 & 旧形式 & 単位 & 物理量 \\\\
\\hline
EOF

for line in ARGF
	line.chomp!
	word = line.split(/,/)
	for token in word
		if /^\".*\"$/ =~ token then
			token.sub!(/^\"/, '')
			token.sub!(/\"$/, '')
		end
	end
	next if /GRIB/ =~ word[0]
#	break if word[0].empty?
	gvd = (/\*/ =~ word[2]) ? 'GVD' : ''
	word[3] = word[3].gsub(/ +/, "\\,")
	print "{\\tt #{word[1]}} & #{word[0]} & #{gvd} & #{word[3]} & #{word[4]} \\\\\n"
end

print <<EOF
\\hline
\\end{longtable}
EOF
