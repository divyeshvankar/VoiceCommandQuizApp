import { useEffect, useRef, useState, useCallback } from 'react';
import VoiceInput from '../components/VoiceInput';
import AudioPlayer from '../components/AudioPlayer';

const Home = () => {
  const socketRef = useRef(null);
  const [audioUrl, setAudioUrl] = useState('');
  const [currentQuestion, setCurrentQuestion] = useState('');
  const [feedback, setFeedback] = useState('');

  useEffect(() => {
    console.log("Home: Initializing WebSocket connection.");

    if (!socketRef.current) {
      socketRef.current = new WebSocket('ws://172.28.208.110:9001');

      socketRef.current.onopen = () => {
        console.log('Home: Connected to WebSocket server');
      };

      socketRef.current.onmessage = (event) => {
        const message = event.data;
        console.log('Home: Message from server:', message);

        if (message === "Correct!" || message === "Incorrect, try again." || message === "Quiz complete!") {
          setFeedback(message);
          console.log('Home: Updated feedback:', message);
        } else if (message.includes('.wav')) {
          setAudioUrl(`/audio/${message}`);
          console.log('Home: Updated audio URL for playback:', `/audio/${message}`);
        } else {
          setCurrentQuestion(message);
          setFeedback('');
          console.log('Home: Updated question:', message);
        }
      };

      socketRef.current.onerror = (error) => {
        console.error('Home: WebSocket error:', error);
      };

      socketRef.current.onclose = () => {
        console.log('Home: WebSocket connection closed');
        socketRef.current = null;
      };
    }
  }, []);

  const sendMessage = useCallback((message) => {
    console.log('Home: Preparing to send message:', message);

    if (socketRef.current) {
      console.log('Home: WebSocket state:', socketRef.current.readyState);

      if (socketRef.current.readyState === WebSocket.OPEN) {
        console.log('Home: WebSocket is open. Sending message to server:', message);
        socketRef.current.send(message);
      } else {
        console.warn('Home: WebSocket is not open. Current state:', socketRef.current.readyState);
      }
    } else {
      console.warn('Home: WebSocket is not initialized.');
    }
  }, [socketRef]);

  return (
    <div>
      <h1>Voice Command Quiz</h1>
      <p>Current Question: {currentQuestion}</p>
      <p>Feedback: {feedback}</p>
      <VoiceInput sendMessage={sendMessage} />
      <AudioPlayer audioUrl={audioUrl} />
    </div>
  );
};

export default Home;
