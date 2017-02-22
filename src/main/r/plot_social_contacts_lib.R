#############################################################################
#  This file is part of the Stride software. 
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or any 
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Copyright 2016, Willem L, Kuylen E & Broeckhove J
#############################################################################


plot_social_contacts <- function(data_tag,project_dir)
{
  
  
  summary_data_all	<- read.table(paste(data_tag,'_summary.csv',sep=''),header=TRUE,sep=",",stringsAsFactors=F)
  
  ######################
  ## GET DATA       ##
  ######################
  
  exp_dir <- './experiments'
  exp_files <- dir(exp_dir)
  exp_cnt_files <- exp_files[grepl('_contacts.csv',exp_files)]
  
  i_file <- 1
  if(length(exp_cnt_files)>0)
  for(i_file in 1:length(exp_cnt_files))
  {
   
    exp_tag       <- paste0('exp',i_file-1)
    exp_path      <- paste0('./experiments/',exp_tag)
    
    cdata         <- read.table(paste0(exp_path,'_contacts.csv'),sep=',',header=T)
    pdata         <- read.table(paste0(exp_path,'_participants.csv'),sep=',',header=T)
    summary_data  <- summary_data_all[i_file,]
    num_days      <- as.double(summary_data$num_days[i_file])
    
    dim(cdata)
    sum(cdata$cnt_home)
    sum(cdata$cnt_school)
    sum(cdata$cnt_work)
    sum(cdata$cnt_prim_comm)
    
    
    if(dim(cdata)[1]>0 && dim(pdata)[1]>0)
    {
      
      ## people without contacts
      dim(pdata)[1] - length(unique(cdata$local_id))
      flag <- pdata$local_id %in% unique(cdata$local_id)
      table(flag)
      pdata[!flag,]
      dim(pdata)[1] - length(unique(cdata$local_id[cdata$cnt_home==1]))
      
      ## SETTINGS 
      L <- 80
      
      flag <- cdata$part_age>=0 
      tag <- 'debug'
      flag <- cdata$cnt_home==1
      plot_cnt_matrix <- function(flag,tag,num_days)
      {
        # count contacts
        mij_tbl <- table(cdata$part_age[flag],cdata$cnt_age[flag])
        row_ind <- as.numeric(row.names(mij_tbl)) +1 # age 0 == index 1
        col_ind <- as.numeric(colnames(mij_tbl))  +1 # age 0 == index 1
        mij <- matrix(0,max((L+1),row_ind),max((L+1),col_ind))  
        mij[row_ind,col_ind] <- mij_tbl
        
        # count participant per age
        if(sum(mij)==0)
        {
          row_ind <- 1:(L+1)
          col_ind <- 1:(L+1)
        }
        pdata_flag <- pdata$part_age %in% ((min(row_ind,col_ind):max(row_ind,col_ind))-1)
        p_ages_tbl <- table(pdata$part_age[pdata_flag])
        #tbl <- table(cdata$local_id[flag],cdata$part_age[flag])
        #p_ages_tbl <- colSums(tbl>0)
        row_ind <- as.numeric(names(p_ages_tbl)) +1
        p_ages <- matrix(0,max((L+1),row_ind),1)  
        p_ages[row_ind] <- p_ages_tbl
        
        # remove age>L (age_column L+1)  
        mij <- mij[1:(L+1),1:(L+1)]
        p_ages <- p_ages[1:(L+1)]
        
        # adjust for number of participants (if age is present)
        ages_present <- p_ages>0
        p_ages[ages_present]
        mij[ages_present,ages_present] <- mij[ages_present,ages_present]/rep(p_ages[ages_present],each=sum(ages_present)) ## CORRECT??!
        
        mx <- mij[1:5,1:5]
        px <- p_ages[1:5]
        mx[1,]/p_ages[1:5]
        # make mij reciproce
        for(i in 1:L)
        {
          for(j in i:L)
          {
            mij[i,j] <- (mij[i,j] + mij[j,i])/2
            mij[j,i] <- mij[i,j]
          }
        }
        
        # account for multiple days
          mij <- mij/num_days
        
        
        # plot matrix 
        image(c(0:L),c(0:L),mij,xlab="age of respondent",ylab="age of contact",main=paste(tag))
        
        # plot number of contacts
        num_cnt_age <- colSums(mij)
        plot(num_cnt_age,main=paste(tag),xlab='age',ylab='count')  
        
        return(mij)
      }
      
      pdf(paste0(data_tag,'_cnt_',exp_tag,'.pdf'))
      par(mfrow=c(2,2))
      ## TOTAL
      mij_total <- plot_cnt_matrix(cdata$part_age>=0,'total',num_days)
      
      ## HOUSEHOLD
      mij_hh <- plot_cnt_matrix(cdata$cnt_home==1,'household',num_days)
      
      ## SCHOOL
      mij_school <- plot_cnt_matrix(cdata$cnt_school==1,'school',num_days)
      
      ## WORK
      mij_work <- plot_cnt_matrix(cdata$cnt_work==1,'work',num_days)
      
      ## PRIMARY COMMUNITY
      mij_prim_comm <- plot_cnt_matrix(cdata$cnt_prim_comm==1,'prim_comm',num_days)
      dim(mij_prim_comm)
      
      ## SECUNDARY COMMUNITY
      mij_sec_comm <- plot_cnt_matrix(cdata$cnt_sec_comm==1,'sec_comm',num_days)
      dim(mij_sec_comm)
      
      ## LOAD SURVEY DATA FROM FLANDERS AND FULLY CONNECTED HOUSEHOLDS
      survey_mij_hh         <- read.table(file=paste0(project_dir,'/data/ref_miami_household_gam_mij_rec.csv'),sep=';',dec=',',header=T)
      survey_mij_school     <- read.table(file=paste0(project_dir,'/data/ref_fl2010_regular_weekday_school_gam_mij_rec.csv'),sep=';',dec=',',header=T)
      survey_mij_work       <- read.table(file=paste0(project_dir,'/data/ref_fl2010_regular_weekday_workplace_gam_mij_rec.csv'),sep=';',dec=',',header=T)
      survey_mij_community  <- read.table(file=paste0(project_dir,'/data/ref_fl2010_regular_weekday_community_gam_mij_rec.csv'),sep=';',dec=',',header=T)
      survey_mij_total      <- survey_mij_hh + survey_mij_work + survey_mij_school + survey_mij_community
      
      survey_mij_school_weekend    <- survey_mij_school*0
      survey_mij_work_weekend      <- survey_mij_work*0
      survey_mij_community_weekend  <- read.table(file=paste0(project_dir,'/data/ref_fl2010_weekend_community_gam_mij_rec.csv'),sep=';',dec=',',header=T)
      survey_mij_total_weekend     <- survey_mij_hh + survey_mij_community_weekend 
      
      ## COMPARE
      par(mfrow=c(3,2))
      
      plot(colSums(survey_mij_total),main='total',xlab='age',ylab='contacts',type='l')
      lines(colSums(survey_mij_total_weekend),main='total',xlab='age',ylab='contacts',type='l',lty=2)
      points(colSums(mij_total),col=2)
      legend('topright',c('week','weekend','model'),col=c(1,1,2),lty=c(1,2,0),pch=c(-1,-1,1),cex=0.8)
      
      plot(colSums(survey_mij_hh),main='household',xlab='age',ylab='contacts',type='l')
      points(colSums(mij_hh),col=2)
      legend('topright',c('week','weekend','model'),col=c(1,1,2),lty=c(1,2,0),pch=c(-1,-1,1),cex=0.8)
      
      plot(colSums(survey_mij_school),main='school',xlab='age',ylab='contacts',type='l',ylim=c(-0.1,20))
      lines(colSums(survey_mij_school_weekend),type='l',lty=2)
      points(colSums(mij_school),col=2)
      legend('topright',c('week','weekend','model'),col=c(1,1,2),lty=c(1,2,0),pch=c(-1,-1,1),cex=0.8)
      
      plot(colSums(survey_mij_work),main='work',xlab='age',ylab='contacts',type='l',ylim=c(-0.1,20))
      lines(colSums(survey_mij_work_weekend),type='l',lty=2)
      points(colSums(mij_work),col=2)
      legend('topright',c('week','weekend','model'),col=c(1,1,2),lty=c(1,2,0),pch=c(-1,-1,1),cex=0.8)
      
      plot(colSums(survey_mij_community),main='primary community',xlab='age',ylab='contacts',type='l',ylim=c(-0.1,15))
      lines(colSums(survey_mij_community_weekend),type='l',lty=2)
      points(colSums(mij_prim_comm),col=2)
      legend('topright',c('week','weekend','model'),col=c(1,1,2),lty=c(1,2,0),pch=c(-1,-1,1),cex=0.8)
      
      plot(colSums(survey_mij_community),main='secondary community',xlab='age',ylab='contacts',type='l',ylim=c(-0.1,15))
      lines(colSums(survey_mij_community_weekend),type='l',lty=2)
      points(colSums(mij_sec_comm),col=2)
      legend('topright',c('week','weekend','model'),col=c(1,1,2),lty=c(1,2,0),pch=c(-1,-1,1),cex=0.8)
      
      dev.off()
      
      print(paste('PLOT SOCIAL CONTACTS COMPLETE for:', data_tag, exp_path))
      
      # end if-clause 
    } 
    
  } # end for(logfiles)
  
}




