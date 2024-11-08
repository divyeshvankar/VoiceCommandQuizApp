import { useState, useEffect } from 'react';

const VoiceInput = ({ sendMessage }) => {
  const [listening, setListening] = useState(false);
  const [recognition, setRecognition] = useState(null);

  useEffect(() => {
    if ('webkitSpeechRecognition' in window) {
      const speechRecognition = new window.webkitSpeechRecognition();
      speechRecognition.lang = 'en-US';
      speechRecognition.onresult = (event) => {
        const transcript = event.results[0][0].transcript;
        sendMessage(transcript);
      };
      setRecognition(speechRecognition);
    }
  }, []);

  const startListening = () => {
    if (recognition) {
      setListening(true);
      recognition.start();
    }
  };

  return (
    <button onClick={startListening}>
      {listening ? 'Listening...' : 'Start Voice Command'}
    </button>
  );
};

export default VoiceInput;
