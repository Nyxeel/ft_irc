char buffer[512]; 
int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

if (bytesReceived > 0) {
    buffer[bytesReceived] = '\0';
    
    std::vector<IrcMessage> messages = parser.processBuffer(clientFd, buffer);
    
    for (size_t i = 0; i < messages.size(); ++i) {
        const IrcMessage& msg = messages[i];
        
        if (msg.command == "KICK") {
            // executeKick(clientFd, msg.params);
        } else if (msg.command == "JOIN") {
            // executeJoin(clientFd, msg.params);
        }
        // ... usw.
    }
} else if (bytesReceived == 0) {
    parser.clearClient(clientFd); // Puffer aufräumen!
    close(clientFd);
}
