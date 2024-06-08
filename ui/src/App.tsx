import './App.css'
import { Navbar } from "./components/Navbar/Navbar.tsx";
import {WebsocketProvider} from './providers/WebsocketProvider';
import {DeviceInfo} from "./components/device-info/DeviceInfo.tsx";
import {AccordionList} from "@tremor/react";
import React from "react";
import {RecorderContext, RecorderProvider} from "./providers/RecorderProvider.tsx";
import {AudioInfo} from "./components/audio-info/AudioInfo.tsx";
import {AudioStream} from "./components/audio-stream/AudioStream.tsx";

function App() {
    const [selectedRecorder,] = React.useContext(RecorderContext);

    return (
        <>
            <Navbar />
            <AccordionList>
                <DeviceInfo />
                {
                    selectedRecorder === "Audio Recorder"
                        ? <>
                            <AudioInfo />
                            <AudioStream />
                        </>
                        : <div></div>
                }
            </AccordionList>
        </>
    )
}

export default App
