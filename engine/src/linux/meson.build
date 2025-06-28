# Subsystems
linux_deps = []
if linux_windowing.length() < 1
	error('At least one Linux windowing system must be selected!')
endif
foreach lw : linux_windowing
	subdir(lw)
endforeach
foreach lwd : linux_deps
	windowing_deps += lwd
endforeach