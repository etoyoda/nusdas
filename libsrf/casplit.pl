@Common = ();
%Created = ();
%Opened = ();
$Output = undef;

while (<>) {
    if (/^\#common/) {
	push(@Common, "#line $. \"$ARGV\"\n");
	$Output = undef;
    } elsif (/^\#output +(\S+)/) {
	$candidate = $1;
	if ($Output) {
	    &output_wrap_file;
	}
	$Output = $candidate;
	unless ($Created{$Output}) {
	    unless (open($Output, ">$Output")) {
		warn "$Output: $!\n";
		next;
	    }
	    foreach $line (@Common) {
		print $Output $line;
	    }
	    $Created{$Output} = 1;
	} else {
	    unless ($Opened{$Output}) {
		unless (open($Output, ">>$Output")) {
		    warn "$Output: $!\n";
		    next;
		}
		$Opened{$Output} = 1;
	    }
	}
	print $Output "#line $. \"$ARGV\"\n";
    }elsif(/^\#/){
	if(! $Output){
	    push(@Common, $_);
	}
	else{
	    push(@Local, $_);
	}
    } else {
	if (! $Output){
	    push(@Common, $_);
	}
	else{
	    push(@body, $_);
	}
    }
}
#output last item
if ($Output) {
    &output_wrap_file;
}


exit 0;


sub output_wrap_file{
    print $Output @Local;
    for ($i = 0; $i < 3; $i++){
	foreach $line (@body){
	    if ($line =~ /fortran:([A-Za-z]\w*)/) {
		$pre = $`;
		$kwd = $1;
		$post = $`;
		$pre =~ s/\s*//;  $pre = "$pre\n" if $pre;
		$post =~ s/\s*//;  $post = "$post\n" if $post;
		if($i == 0){
		    $kwd =~ tr/a-z/A-Z/; 
		}
		elsif($i == 1){
		    $kwd =~ tr/A-Z/a-z/;
		    $kwd = $kwd."_";
		}
		elsif($i == 2){
		    $lwrkwd = $kwd;
		    $lwrkwd =~ tr/A-Z/a-z/;			    
		    if(index($lwrkwd, "_") != 0){
			$kwd = $lwrkwd."__";
		    }
		    else{
			$kwd = $lwrkwd."_";
		    }
		}
		print $Output $kwd;
		print $Output "\n";
	    }
	    else{
		print $Output $line;
	    }
	}
    }
    @body = ();
    @Local = ();
    close($Output);
    $Opened{$Output} = 0;
}


