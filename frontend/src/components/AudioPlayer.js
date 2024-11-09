import { useEffect, useRef } from 'react';

const AudioPlayer = ({ audioUrl }) => {
  const audioRef = useRef(null);

  useEffect(() => {
    if (audioUrl && audioRef.current) {
      console.log("AudioPlayer: New audio URL received:", audioUrl);
      audioRef.current.src = audioUrl;
      audioRef.current.play().then(() => {
        console.log("AudioPlayer: Audio playback started.");
      }).catch((error) => {
        console.error("AudioPlayer: Error during playback:", error);
      });
    }
  }, [audioUrl]);

  return <audio ref={audioRef} controls style={{ display: 'none' }} />;
};

export default AudioPlayer;
