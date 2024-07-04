import './App.css'
import { Navbar } from "./components/Navbar/Navbar.tsx";
import {DeviceInfo} from "./components/device-info/DeviceInfo.tsx";
import {AccordionList} from "@tremor/react";
import React from "react";
import {RecorderContext, RecorderProvider} from "./providers/RecorderProvider.tsx";
import {AudioInfo} from "./components/audio-info/AudioInfo.tsx";
import {AudioStream} from "./components/audio-stream/AudioStream.tsx";
import {Route, BrowserRouter as Router, Routes} from "react-router-dom";
import {LoginPage} from "./components/login/Login.tsx";
import {AdminConfig} from "./components/admin-config/AdminConfig.tsx";

function App() {
    const [selectedRecorder,] = React.useContext(RecorderContext);

    return (
        <Router>
            <Routes>
                <Route path="/" element={<>
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
                </>} />
                <Route path="/admin" element={<>
                    <Navbar />

                    <AccordionList>
                        <DeviceInfo />
                        {
                            selectedRecorder === "Audio Recorder"
                                ? <>
                                    <AudioInfo />
                                    <AudioStream />
                                    <AdminConfig />
                                </>
                                : <div></div>
                        }
                    </AccordionList>
                </>} />
                <Route path="/auth" element={
                    <>
                        <h1 className={"text-7xl max-w-fit mx-auto"}>Authorization successful!</h1>
                    </>
                } />
                <Route path="/login" element={<LoginPage/>} />
            </Routes>
        </Router>
    )
}

export default App
