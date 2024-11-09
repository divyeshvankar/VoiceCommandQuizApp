// VoiceInput.js (Updated settings)
import { useState, useEffect } from 'react';

const VoiceInput = ({ sendMessage }) => {
  const [listening, setListening] = useState(false);
  const [recognition, setRecognition] = useState(null);

  useEffect(() => {
    if ('webkitSpeechRecognition' in window) {
      const speechRecognition = new window.webkitSpeechRecognition();
      speechRecognition.lang = 'en-US';
      speechRecognition.continuous = true; // Allow continuous recognition
      speechRecognition.interimResults = true; // Capture interim results

      // Log when starting to listen
      speechRecognition.onstart = () => {
        console.log("VoiceInput: Speech recognition started.");
        setListening(true);
      };

      // Capture result
      speechRecognition.onresult = (event) => {
        try {
          if (event.results.length > 0) {
          const transcript = event.results[0][0].transcript;
          console.log("VoiceInput: Transcript captured:", transcript);
          setListening(false);

          // Directly calling sendMessage with transcript
          console.log("VoiceInput: Attempting to send message with transcript:", transcript);
          sendMessage(transcript);
          }else {
            console.warn("VoiceInput: No transcript available in results");
        }

        } catch (error) {
          console.error("VoiceInput: Error during onresult:", error);
        }
      };

      // Handle the end of recognition
      speechRecognition.onend = () => {
        console.log("VoiceInput: Speech recognition ended.");
        setListening(false);
        console.log("VoiceInput: After recognition ended and setListening false.");

        // Fallback to ensure sendMessage is called if no transcript was captured
        setTimeout(() => {
          if (!listening) {
            console.log("VoiceInput: No transcript captured, sending fallback message 'No response'.");
            sendMessage("No response");
          }
        }, 1500);  // Increased delay to ensure recognition has fully ended
      };

      // Log when speech starts being detected
      speechRecognition.onspeechstart = () => {
        console.log("VoiceInput: Speech detected.");
      };

      // Log when speech ends
      speechRecognition.onspeechend = () => {
        console.log("VoiceInput: Speech ended.");
      };

      // Log when sound starts (including background noise)
      speechRecognition.onsoundstart = () => {
        console.log("VoiceInput: Sound detected.");
      };

      // Log when sound ends (including background noise)
      speechRecognition.onsoundend = () => {
        console.log("VoiceInput: Sound ended.");
      };

      // Log any errors encountered
      speechRecognition.onerror = (event) => {
        console.error("VoiceInput: Speech recognition error:", event.error);
        if (event.error === 'network') {
            console.warn("VoiceInput: Network error - retrying connection.");
            setTimeout(() => {
                speechRecognition.start();  // Restart correctly with the same instance
            }, 2000);
        }
        setListening(false);
    };
    

      setRecognition(speechRecognition);
    } else {
      console.warn("VoiceInput: Speech recognition not supported in this browser.");
    }
  }, [sendMessage]);

  const startListening = () => {
    if (recognition && !listening) {
        console.log("VoiceInput: Starting to listen for voice input.");
        setListening(true);  // Immediately set listening to true
        recognition.start();
        
        setTimeout(() => {
            if (listening) {
                console.warn("VoiceInput: Manually stopping recognition due to inactivity.");
                recognition.stop();
            }
        }, 10000);
    } else {
        console.log("VoiceInput: Already listening or recognition not available.");
    }
};


  return (
    <div>
      <button onClick={startListening}>
        {listening ? 'Listening...' : 'Start Voice Command'}
      </button>
      <button onClick={() => {
        console.log("VoiceInput: Manually sending 'four'.");
        sendMessage("four");
      }}>
        Send "four" Manually
      </button>
    </div>
  );
};

export default VoiceInput;
