(*
Prints two labels containing the projector LED factory setting.
*)

tell application "DYMO Label"
	
	openLabel in "EmptyAddressLabel.label"
	
	set ledcurrentfile to ("tmp:LEDCurrent.txt")
	set theFileContents to (read file ledcurrentfile)
	
	set txt to a reference to last item of print objects
	tell txt
		set content to "ProjectorLEDCurrent = " & theFileContents
	end tell
	
	set elem to a reference to last element
	tell elem
		set xPosition to 50
		set yPosition to 10
		set width to 150
		set height to 50
	end tell
	
	redrawLabel
	printLabel
	
end tell
