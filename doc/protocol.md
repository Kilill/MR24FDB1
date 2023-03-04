# Over the line protocoll
the packet structure as seen from the serial interface (9600,8Bits No parity,1 Stop bit)

\image html MR24FDB1_page1.svg width=600

# Get command packets

they send a request for information to the module, the module will resopond with a responce packet

\image html MR24FDB1_get.svg width=900

# Set commands packets

Set commands sends parameters and data to the the module

\image html MR24FDB1_set.svg width=900

# Responce packets

Sent from the module as a responce to either a Get command or a Set command

\image html MR24FDB1_responce.svg width=900

# Report packets

Reports sent in intervalls by the module

\image html MR24FDB1_reports.svg width=900

# Fall packets

Sent by the module if the fall function is enabled and a fall is detected

\image html MR24FDB1_fall.svg width=900


