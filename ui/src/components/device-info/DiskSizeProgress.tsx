import { Card, ProgressBar, Text, Title } from '@tremor/react';

// eslint-disable-next-line @typescript-eslint/ban-ts-comment
// @ts-expect-error
const DiskSizeProgress = ({ totalSize, usedSize }) => {

    const usedPercentage: number = (usedSize / totalSize) * 100;

    return (
        <Card className={"mx-auto"} style={{width: "20vw"}}>
            <Title>Disk Usage</Title>
            <Text>Total Size: {totalSize} GB</Text>
            <Text>Used Size: {usedSize} GB</Text>
            <ProgressBar value={usedPercentage} />
        </Card>
    );
};

export default DiskSizeProgress;