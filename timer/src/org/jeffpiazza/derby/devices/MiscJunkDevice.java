package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.Timestamp;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class MiscJunkDevice extends TimerDeviceTypical {
  public MiscJunkDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  public static String toHumanString() {
    return "MiscJunk PDT (http://www.miscjunk.org/mj/pg_pdt.html)";
  }

  private int numberOfLanes = 0;
  private ArrayList<Message.LaneResult> results;

  private static final String READ_VERSION = "V";
  private static final String READ_LANE_COUNT = "N";
  private static final String RESET = "R";
  private static final String MASK_LANE = "M";
  private static final String UNMASK_ALL_LANES = "U";
  private static final String POLL_GATE = "G";
  private static final String FORCE_END_OF_RACE = "F";

  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE,
                                   /* rts */ false,
                                   /* dtr */ false)) {
      return false;
    }

    portWrapper.write(READ_VERSION);
    long deadline = System.currentTimeMillis() + 500;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("vert=") >= 0) {

        // Responds either "P" or "O"
        portWrapper.writeAndWaitForResponse(RESET, 500);

        String nl = portWrapper.writeAndWaitForResponse(READ_LANE_COUNT, 500);
        int nl_index = nl.indexOf("numl=");
        if (nl_index >= 0 && nl.length() > nl_index + 5) {
          this.numberOfLanes = nl.charAt(nl_index + 5) - '0';
          if (0 < numberOfLanes && numberOfLanes <= 9) {
            portWrapper.logWriter().serialPortLogInternal(
                Timestamp.string() + ": "
                + this.numberOfLanes + " lane(s) reported.");
            setUp();
            return true;
          }
        }
      }
    }

    return false;
  }

  private static final Pattern resultLine
      = Pattern.compile("(\\d) - (\\d+\\.\\d+)");

  private void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        if (line.equals("B")) {
          portWrapper.logWriter().serialPortLogInternal("Detected gate opening");  // TODO
          // Sent when the gate opens
          onGateStateChange(false);
          return "";
        }
        Matcher m = resultLine.matcher(line);
        if (m.find()) {
          int lane = Integer.parseInt(m.group(1));
          portWrapper.logWriter().serialPortLogInternal(
              "Detected result for lane " + lane + " = " + m.group(2));  // TODO
          if (results != null) {
            TimerDeviceUtils.addOneLaneResult(lane, m.group(2), -1, results);
            // Results are sent in lane order, so results are complete when
            // we see an entry for the last lane.
            if (lane == numberOfLanes) {
              raceFinished((Message.LaneResult[]) results.toArray(
                  new Message.LaneResult[results.size()]));
              results = null;
            }
          } else {
            portWrapper.logWriter().serialPortLogInternal(
                "*** Unexpected lane result");
          }
          return "";
        } else {
          return line;
        }
      }
    });
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  @Override
  protected void maskLanes(int lanemask) throws SerialPortException {
    portWrapper.writeAndDrainResponse(UNMASK_ALL_LANES, 2, 100);
    for (int lane = 0; lane < getSafeNumberOfLanes(); ++lane) {
      if ((lanemask & (1 << lane)) == 0) {
        portWrapper.writeAndDrainResponse(MASK_LANE + (char) ('1' + lane), 2, 100);
      }
    }
  }

  @Override
  protected boolean interrogateGateIsClosed() throws NoResponseException,
                                                     SerialPortException,
                                                     LostConnectionException {
    portWrapper.write(POLL_GATE);
    long deadline = System.currentTimeMillis() + 500;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("O") >= 0) {
        return false;
      }

      if (s.indexOf(".") >= 0) {
        return true;
      }
    }
    throw new NoResponseException();
  }

  @Override
  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      // Upon entering RESULTS_OVERDUE state, we sent FORCE_END_OF_RACE; see
      // onTransition.
      if (portWrapper.millisSinceLastContact() > 1000) {
        throw new LostConnectionException();
      } else if (rsm.millisInCurrentState() > 1000) {
        giveUpOnOverdueResults();
      }
    }
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
      throws SerialPortException {
    if (newState == RacingStateMachine.State.MARK) {
      results = new ArrayList<Message.LaneResult>();
    } else if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      portWrapper.write(FORCE_END_OF_RACE);
      logOverdueResults();
    }
  }
}
