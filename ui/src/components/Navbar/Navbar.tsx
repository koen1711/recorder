import {Button, Dialog, DialogPanel, Select, SelectItem} from '@tremor/react';
import React from 'react';
import {WebsocketContext} from "../../providers/WebsocketProvider.tsx";
import {RecorderContext} from "../../providers/RecorderProvider.tsx";
import {Config} from "./Config.tsx";
import {PermissionContext} from "../../providers/PermissionProvider.tsx";


export function Navbar() {
    const [, val, send] = React.useContext(WebsocketContext);
    const [allowUnregisteredRecording,,allowUnregisteredConfig] = React.useContext(PermissionContext);
    // @ts-ignore
    const [isConfigOpen, setIsConfigOpen]: [boolean|null, (arg0: boolean|null) => void] = React.useState(false);
    const [returnConfig, setReturnConfig] = React.useState({});
    // @ts-ignore
    const [recorderConfig, setRecorderConfig] = React.useState(null);
    const [recorders, setRecorders] = React.useState({type: 'recorders', recorders: []});
    const [selectedRecorder, setSelectedRecorder, recording, setRecording] = React.useContext(RecorderContext);
    const [configMistake, setConfigMistake] = React.useState("");

    React.useEffect(() => {
        if (!val) {
            return;
        }
        const json = JSON.parse(val);
        if (json.type === 'recorders') {
            setRecorders(json);
            if (json["selectedRecorder"] !== undefined) {
                setSelectedRecorder(json["selectedRecorder"]);
            }
        } else if (json.type === 'recorder-selected') {
            setSelectedRecorder(json.recorder);
        } else if (json.type === 'config') {
            setReturnConfig({
                type: "configure",
                config: {}
            })
            setRecorderConfig(json);
        } else if (json.type === 'valid-config') {
            setIsConfigOpen(false);
        } else if (json.type === 'invalid-config') {
            setConfigMistake(json.error);
            setIsConfigOpen(true);
        } else if (json.type === 'recording-status') {
            if (json.recording) {
                setRecording(true);
            }
            if (json.selectedRecorder !== undefined) {
                setSelectedRecorder(json.selectedRecorder);
            }
            if (json.recorders !== undefined) {
                setRecorders(json);
            }
        }

    }, [val]);

    React.useEffect(() => {
        if (isConfigOpen === true) {
            send(JSON.stringify({type: 'query-config'}));
        }
    }, [isConfigOpen]);

    React.useEffect(() => {
        if (recording == null) {
            send(JSON.stringify({type: 'start-recording'}));
        } else if (!recording) {
            send(JSON.stringify({type: 'stop-recording'}));
        }
    }, [recording])


    function updateConfig(stack: string[], value: string) {
        if (recorderConfig == null) {
            return;
        }
        const config = recorderConfig;
        // so the stack says in what key we are, for example ["section", "section", "section", "section"]
        // so we need to go to the last key and set the value there
        let current = config;
        for (let i = 0; i < stack.length; i++) {
            if (current[stack[i]] === undefined) {
                throw new Error("Invalid path: " + stack.slice(0, i + 1).join('.'));
            }
            current = current[stack[i]];
        }

        // Update the value at the final key in the stack
        // @ts-ignore
        current["value"] = value;

        const returnConf = returnConfig;
        // @ts-ignore
        returnConf.config[stack[stack.length - 1]] = value

        setReturnConfig(returnConf)
        setRecorderConfig(config);
    }

    function handleRecordingChange() {
        if (!recording) {
            setRecording(null);
        } else if (recording) {
            setRecording(false)
        }
    }

    return (
        <div className={"flex items-center p-4 navbar"}>
            <h1>Remote Control Dashboard</h1>
            {recorders["recorders"] == null ? null : <Select className={"w-1/12 mx-auto"} value={selectedRecorder} onValueChange={(val) => {setSelectedRecorder(val); send(JSON.stringify({type: 'select-recorder', recorder: val}));} }>
                {recorders["recorders"].map((recorder: string) => <SelectItem key={recorder} value={recorder}></SelectItem>)}
            </Select>}

            <Button disabled={selectedRecorder == "" || !allowUnregisteredRecording} className={"ml-4"}
                    loading={recording == null}
                    color={recording ? "red" : "green"}
                    onClick={ () => handleRecordingChange()}
            >{recording ? "Recording" : (!recording ? "Start recording" : "Starting...")}</Button>
            <Button disabled={selectedRecorder == "" || !allowUnregisteredConfig} className={"ml-4"} onClick={() => setIsConfigOpen(true)}>Config</Button>

            <style>
                {/*    Align config-button on the right side of the nav bar */}
                {`
                .navbar {
                    justify-content: space-between;
                    background-color: #333;
                    color: white;
                }
                `}


            </style>

            <Dialog open={isConfigOpen == true} onClose={() => setIsConfigOpen(null)} static={true}>
                <DialogPanel>
                    <h1 className="mb-6 text-xl">Configuration</h1>
                    <Config config={recorderConfig} updateConfig={updateConfig} configMistake={configMistake}>

                    </Config>
                    <div className={"w-full h-1"}></div>
                    <Button className="mt-8 w-full" onClick={() => { setIsConfigOpen(false); send(JSON.stringify(returnConfig)); }}>
                        Save
                    </Button>

                </DialogPanel>

            </Dialog>
        </div>
    )
}

