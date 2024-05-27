## Offline Messenger
Application made in C to demonstrate the functionality of a TCP socket connection. Its purpose is to have as many users as possible being able to chat with eachother "live" as well as having the possibility to message offline users. Upon connecting to the server, the "offline" user will be shown messages received while being offline. For each user connected, the server makes a new thread. Users send commands through the TCP socket to the server which processes them and sends back the response desired to the user (for example, the user wants to see who is online, it sends the command to the server, the server searches in the database who is connected and returns a list to the user). The database is a SQLite database, as it fits the small scope of the project.