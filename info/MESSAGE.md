* STANDART OF MESSAGES (ILLUM NETWORK)


| Type of message | Public certificate| Additional inforamtion | Text |
|--------------------------|-------------------------------|------------------------------------|------------------:|
|1 byte|32 bytes|467 bytes|9000 bytes|

1) Type of message (Obligatorily):
	* System message - For supporting routing of network.
	* User message - Message for some othe client node.

2) Public certificate (Obligatorily):
	* Unique identificator of user and key for encryption of messages.

3) Additional inforamtion (Obligatorily):
	* Here can be some information which helps to work some types of
	messages.

4) Text (Optionality):
	* User text for some other client of network.

