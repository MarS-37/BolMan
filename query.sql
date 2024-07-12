SELECT PDivision.Name, pList.ID, pList.Name, pList.FirstName, pList.MidName, pLogData.TimeVal, pLogData.Event, pLogData.Mode, pLogData.Remark
FROM pLogData, pList, PDivision
WHERE pLogData.HozOrgan = pList.ID AND
      pLogData.TimeVal>='<<MinDate>>' AND
      pLogData.TimeVal <'<<MaxDate>>' AND
      pLogData.Event = 32 AND
      PDivision.ID = pList.Section
ORDER BY pList.Name, pList.FirstName, pList.MidName, pLogData.TimeVal