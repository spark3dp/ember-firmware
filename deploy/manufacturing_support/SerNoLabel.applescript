(*
Prints two labels containing the projector LED factory setting.
*)

tell application "DYMO Label"
	
	openLabel in "EmptyProjectorLabel.label"
	
	set ledcurrentfile to ("tmp:LEDCurrent.txt")
	set theFileContents to (read file ledcurrentfile)
	
	set txt to a reference to last item of print objects
	tell txt
		set content to "Projector
LED
Current
   " & theFileContents
	end tell
	
	set elem to a reference to last element
	tell elem
		set xPosition to 8
		set yPosition to -4
		set width to 60
		set height to 100
	end tell
	
	redrawLabel
	printLabel
	
end tell
