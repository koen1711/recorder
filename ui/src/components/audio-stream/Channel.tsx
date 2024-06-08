import {Card, Switch} from "@tremor/react";
import {useState} from "react";


// @ts-ignore
// eslint-disable-next-line @typescript-eslint/no-unused-vars

function Channel({ channel, onChannelClick }) {
    const [checked, setChecked] = useState(false);
    function clickChannel() {
        if (onChannelClick(channel, !checked)) {
            setChecked(!checked);
        }
    }

    return (
        <Card className={"max-w-min ml-1 mt-1 channel-card"}>
            {channel.name}
            <Switch onChange={() => clickChannel()} checked={checked} />
        </Card>
    );
}

export default Channel;