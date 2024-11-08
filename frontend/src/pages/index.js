import { useEffect, useRef, useState } from 'react';
import VoiceInput from '../components/VoiceInput';
import AudioPlayer from '../components/AudioPlayer';

const Home = () => {
  const socketRef = useRef(null);  // useRef to persist WebSocket instance across renders
  const [audioUrl, setAudioUrl] = useState('');

  useEffect(() => {
    // Initialize the WebSocket connection if not already connected
    if (!socketRef.current) {
      socketRef.current = new WebSocket('ws://172.28.208.110:9001');

      socketRef.current.onopen = () => {
        console.log('Connected to WebSocket server');
      };

      socketRef.current.onmessage = (event) => {
        const message = event.data;
        // Check if the message is a URL for audio playback
        if (message.includes('.wav')) {
          setAudioUrl(`/audio/${message}`);
        } else {
          console.log('Message from server:', message);
        }
      };

      socketRef.current.onerror = (error) => {
        console.error('WebSocket error:', error);
      };

      socketRef.current.onclose = () => {
        console.log('WebSocket connection closed');
        socketRef.current = null;  // Reset the socketRef to allow reconnection if needed
      };
    }
  }, []);

  const sendMessage = (message) => {
    // Only send if WebSocket is open
    if (socketRef.current && socketRef.current.readyState === WebSocket.OPEN) {
      console.log('Sending message to server:', message);
      socketRef.current.send(message);
    } else {
      console.warn('WebSocket is not connected');
    }
  };

  return (
    <div>
      <h1>Voice Command Application</h1>
      <VoiceInput sendMessage={sendMessage} />
      <AudioPlayer audioUrl={audioUrl} />
    </div>
  );
};

export default Home;
